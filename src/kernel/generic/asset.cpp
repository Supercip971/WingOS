
#include "asset.hpp"

#include "hw/mem/addr_space.hpp"

core::Result<Asset *> _asset_create(Space *space, AssetKind kind)
{
    Asset *asset = new Asset();
    *asset = {};
    asset->kind = kind;

    asset->lock.lock();

    asset->ref_count = 1;

    if (space != nullptr)
    {
        AssetPtr ptr;
        ptr.asset = asset;
        ptr.handle = space->uid++;

        if (space->assets.push(ptr).is_error())
        {
            log::err$("unable to add asset to space");
            delete asset;
            return core::Result<Asset *>::error("asset create: unable to add asset to space");
        }
    }
    return asset;
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

            log::warn$("asset_release: memory asset is not supported yet");
        }
        else if (asset->kind == OBJECT_KIND_MAPPING)
        {
            log::warn$("asset_release: mapping asset is not supported yet");
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

core::Result<Asset *> asset_create_memory(Space *space, AssetMemoryCreateParams params)
{

    if (params.size == 0)
    {
        return core::Result<Asset *>::error("size must be greater than 0");
    }

    if (params.addr != 0 && params.addr + params.size > kernel_virtual_base())
    {
        return core::Result<Asset *>::error("addr must be lower than kernel virtual base");
    }

    Asset *asset = try$(_asset_create(space, OBJECT_KIND_MEMORY));
    asset->memory.size = params.size;
    asset->memory.addr = params.addr;
    core::Result<PhysAddr> res = {};
    if (params.addr == 0)
    {
        if (params.lower_half)
        {
            res = Pmm::the().allocate(params.size, IOL_ALLOC_MEMORY_FLAG_LOWER_SPACE);
        }
        else
        {
            res = Pmm::the().allocate(params.size);
        }
    }
    else
    {
        return core::Result<Asset *>::error("addr must be 0 to allocate memory, non 0 not supported yet");
    }

    if (res.is_error())
    {
        log::err$("asset_create_memory: unable to allocate memory: {}", res.error());

        asset->lock.release();
        asset_release(space, asset);
        return core::Result<Asset *>::error("unable to allocate memory");
    }

    asset->memory.addr = res.unwrap()._addr;

    asset->lock.release();
    return asset;
}

core::Result<Asset *> asset_create_mapping(Space *space, AssetMappingCreateParams params)
{
    Asset *asset = try$(_asset_create(space, OBJECT_KIND_MAPPING));

    if (params.start >= params.end)
    {
        return core::Result<Asset*>::error("asset_create_mapping: start must be less than end");
    }

    if (params.physical_mem->kind != OBJECT_KIND_MEMORY)
    {
        return core::Result<Asset*>::error("asset_create_mapping: physical_mem must be a memory asset");
    }

    if (params.start >= kernel_virtual_base())
    {
        return core::Result<Asset*>::error("asset_create_mapping: start must be less than kernel virtual base");
    }

    asset->mapping.start = params.start;
    asset->mapping.end = params.end;
    asset->mapping.physical_mem = params.physical_mem;
    asset->mapping.writable = params.writable;
    asset->mapping.executable = params.executable;

    if (space == nullptr)
    {
        log::warn$("asset_create_mapping: space is null");
        asset->lock.release();
        return asset;
    }

    auto flags = PageFlags().user(true).executable(asset->mapping.executable).present(true).writeable(asset->mapping.writable);

    if (!space->vmm_space.map({asset->mapping.start, asset->mapping.end},
                              {asset->mapping.physical_mem->memory.addr, asset->mapping.physical_mem->memory.size + asset->mapping.physical_mem->memory.addr},
                              flags))
    {
        asset->lock.release();
        asset_release(space, asset);
        return core::Result<Asset *>::error("unable to map memory");
    }

    asset->lock.release();

    return asset;
}

core::Result<Asset *> asset_create_task(Space *space, AssetTaskCreateParams params)
{
    Asset *asset = try$(_asset_create(space, OBJECT_KIND_TASK));
    asset->task = kernel::Task::task_create().unwrap();
    if (asset->task == nullptr)
    {
        asset->lock.release();
        asset_release(space, asset);
        return core::Result<Asset *>::error("unable to create task asset");
    }

    if (asset->task->_initialize(params.launch, &space->vmm_space).is_error())
    {
        asset->lock.release();
        asset_release(space, asset);
        return core::Result<Asset *>::error("unable to initialize task asset");
    }

    asset->lock.release();
    return asset;
}
