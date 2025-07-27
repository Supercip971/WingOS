
#include "asset.hpp"

#include "arch/x86_64/paging.hpp"
#include "hw/mem/addr_space.hpp"
#include "iol/mem_flags.h"
#include "math/align.hpp"

core::Result<AssetPtr> _asset_create(Space *space, AssetKind kind)
{
    Asset *asset = new Asset();
    *asset = {};
    asset->kind = kind;

    asset->lock.lock();

    asset->ref_count = 1;

    AssetPtr ptr;
    ptr.asset = asset;
    ptr.handle = 0;
    if (space != nullptr)
    {
        ptr.handle = space->uid++;

        if (space->assets.push(ptr).is_error())
        {
            log::err$("unable to add asset to space");
            delete asset;
            return core::Result<AssetPtr>::error("asset create: unable to add asset to space");
        }
    }
    return ptr;
}

void asset_own(Asset *asset)
{
    asset->lock.lock();
    asset->ref_count++;
    asset->lock.release();
}

void asset_release(Space *space, Asset *asset)
{

    if (space != nullptr)
    {
        for (size_t i = 0; i < space->assets.len(); i++)
        {
            if (space->assets[i].asset == asset)
            {
                space->assets.pop(i);
                break;
            }
        }
    }

    asset->lock.lock();
    asset->ref_count--;

    if (asset->ref_count == 0)
    {
        if (asset->kind == OBJECT_KIND_MEMORY)
        {

            Pmm::the().release(PhysAddr{asset->memory.addr}, asset->memory.size);
        }
        else if (asset->kind == OBJECT_KIND_MAPPING)
        {
            if (space != nullptr)
            {
                space->vmm_space.unmap(
                    VirtRange(asset->mapping.start,
                    asset->mapping.end));
            }
        }
        else if (asset->kind == OBJECT_KIND_IPC)
        {
            log::warn$("asset_release: ipc asset is not supported yet");
        }
        else if (asset->kind == OBJECT_KIND_SPACE)
        {
            for (size_t i = 1; i < asset->space->assets.len(); i++)
            {
                asset_release(asset->space, asset->space->assets[i].asset);
            }

            delete asset->space; // delete the space if it exists
        }
        else if (asset->kind == OBJECT_KIND_TASK)
        {
            log::warn$("asset_release: task asset is not supported yet");
        }
    }
    asset->lock.release();

    delete asset;
}

core::Result<AssetPtr> asset_create_memory(Space *space, AssetMemoryCreateParams params)
{

    if (params.size == 0)
    {
        return core::Result<AssetPtr>::error("size must be greater than 0");
    }

    if (params.addr != 0 && params.addr + params.size > kernel_virtual_base())
    {
        return core::Result<AssetPtr>::error("addr must be lower than kernel virtual base");
    }

    AssetPtr ptr = try$(_asset_create(space, OBJECT_KIND_MEMORY));
    ptr.asset->memory.size = params.size;
    ptr.asset->memory.addr = params.addr;
    core::Result<PhysAddr> res = {};
    if (params.addr == 0)
    {
        if (params.lower_half)
        {
            res = Pmm::the().allocate(math::alignUp<size_t>(params.size, arch::amd64::PAGE_SIZE) / arch::amd64::PAGE_SIZE, IOL_ALLOC_MEMORY_FLAG_LOWER_SPACE);
        }
        else
        {
            res = Pmm::the().allocate(math::alignUp<size_t>(params.size, arch::amd64::PAGE_SIZE) / arch::amd64::PAGE_SIZE);
        }
    }
    else
    {
        return core::Result<AssetPtr>::error("addr must be 0 to allocate memory, non 0 not supported yet");
    }

    if (res.is_error())
    {
        log::err$("asset_create_memory: unable to allocate memory: {}", res.error());

        ptr.asset->lock.release();
        asset_release(space, ptr.asset);
        return core::Result<AssetPtr>::error("unable to allocate memory");
    }

    ptr.asset->memory.addr = res.unwrap()._addr;
    

    ptr.asset->lock.release();
    return ptr;
}

core::Result<AssetPtr> asset_create_mapping(Space *space, AssetMappingCreateParams params)
{
    AssetPtr ptr = try$(_asset_create(space, OBJECT_KIND_MAPPING));

    if (params.start >= params.end)
    {
        return core::Result<AssetPtr>::error("asset_create_mapping: start must be less than end");
    }

    if (params.physical_mem->kind != OBJECT_KIND_MEMORY)
    {
        return core::Result<AssetPtr>::error("asset_create_mapping: physical_mem must be a memory asset");
    }

    if (params.start >= kernel_virtual_base())
    {
        return core::Result<AssetPtr>::error("asset_create_mapping: start must be less than kernel virtual base");
    }

    ptr.asset->mapping.start = params.start;
    ptr.asset->mapping.end = params.end;
    ptr.asset->mapping.physical_mem = params.physical_mem;
    ptr.asset->mapping.writable = params.writable;
    ptr.asset->mapping.executable = params.executable;

    if (space == nullptr)
    {
        log::warn$("asset_create_mapping: space is null");
        ptr.asset->lock.release();
        return ptr;
    }

    auto flags = PageFlags()
                     .user(true)
                     .executable(ptr.asset->mapping.executable)
                     .present(true)
                     .writeable(ptr.asset->mapping.writable)
                     ;

    if (!space->vmm_space.map(
            {ptr.asset->mapping.start, ptr.asset->mapping.end},
            {ptr.asset->mapping.physical_mem->memory.addr,
             ptr.asset->mapping.physical_mem->memory.size + ptr.asset->mapping.physical_mem->memory.addr},
            flags))
    {
        ptr.asset->lock.release();
        asset_release(space, ptr.asset);
        return core::Result<AssetPtr>::error("unable to map memory");
    }

    ptr.asset->lock.release();

    return ptr;
}

core::Result<AssetPtr> asset_create_task(Space *space, AssetTaskCreateParams params)
{
    AssetPtr ptr = try$(_asset_create(space, OBJECT_KIND_TASK));
    ptr.asset->task = kernel::Task::task_create().unwrap();

    ptr.asset->task->_space_owner = space;
    if (ptr.asset->task == nullptr)
    {
        ptr.asset->lock.release();
        asset_release(space, ptr.asset);
        return core::Result<AssetPtr>::error("unable to create task asset");
    }

    if (ptr.asset->task->_initialize(params.launch, &space->vmm_space).is_error())
    {
        ptr.asset->lock.release();
        asset_release(space, ptr.asset);
        return core::Result<AssetPtr>::error("unable to initialize task asset");
    }

    ptr.asset->lock.release();
    return ptr;
}
