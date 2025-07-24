
#include "asset.hpp"


Asset * _asset_create(Space* space, AssetKind kind)
{
    Asset *asset = new Asset();
    *asset = {};
    asset->kind = kind;
 
    asset->lock.lock();

    asset->ref_count = 1;


    if(space != nullptr)
    {
        AssetPtr ptr; 
        ptr.asset = asset;
        ptr.handle = space->uid ++;
    
        space->assets.push(ptr);
 
    }
   return asset;
}
void asset_own(Asset *asset)
{
    asset->lock.lock();
    asset->ref_count++;
    asset->lock.release();
}
void asset_release(Asset *asset)
{
    asset->lock.lock();
    asset->ref_count--;
    if (asset->ref_count == 0)
    {
        delete asset;
    }
    asset->lock.release();
}

Asset *asset_create_memory(Space* space, AssetMemoryCreateParams params)
{
    Asset *asset = _asset_create(space, OBJECT_KIND_MEMORY);
    asset->memory.size = params.size;
    asset->memory.addr = params.addr;
    if (params.addr == 0)
    {
        if (params.lower_half)
        {
            asset->memory.addr = Pmm::the().allocate(params.size, IOL_ALLOC_MEMORY_FLAG_LOWER_SPACE).unwrap();
        }
        else
        {
            asset->memory.addr = Pmm::the().allocate(params.size).unwrap();
        }
    }

    asset->lock.release();
    return asset;
}

Asset *asset_create_mapping(Space* space, AssetMappingCreateParams params)
{
    Asset *asset = _asset_create(space, OBJECT_KIND_MAPPING);

    if (params.start >= params.end)
    {
        log::err$("asset_create_mapping: start must be less than end");
        return nullptr;
    }

    if(params.physical_mem->kind != OBJECT_KIND_MEMORY)
    {
        log::err$("asset_create_mapping: physical_mem must be a memory asset");
        return nullptr;
    }

    if (params.start >= kernel_virtual_base())
    {
        log::err$("asset_create_mapping: start must be less than kernel virtual base");

        return nullptr;
    }
    asset->mapping.start = params.start;
    asset->mapping.end = params.end;
    asset->mapping.physical_mem = params.physical_mem;
    asset->mapping.writable = params.writable;
    asset->mapping.executable = params.executable;

    if (space == nullptr)
    {
        log::warn$("asset_create_mapping: space is null");
        return nullptr;
    }

    auto flags = PageFlags().user(true)
                    .executable(asset->mapping.executable)
                    .present(true)
                    .writeable(asset->mapping.writable);

    if (!space->vmm_space.map({asset->mapping.start, asset->mapping.end},
                    {asset->mapping.physical_mem->memory.addr, asset->mapping.physical_mem->memory.size + asset->mapping.physical_mem->memory.addr},
                   flags ))
    {
        log::err$("asset_create_mapping: failed to map memory");
        return nullptr;
    }

    asset->lock.release();

    return asset;
}

Asset* asset_create_task(Space* space, AssetTaskCreateParams params)
{
    Asset *asset = _asset_create(space, OBJECT_KIND_TASK);
    asset->task = kernel::Task::task_create().unwrap();
    if (asset->task == nullptr)
    {
        log::err$("unable to create task asset");
        return nullptr;
    }

    if (!asset->task->_initialize(params.launch, &space->vmm_space))
    {
        log::err$("unable to initialize task asset");
        return nullptr;
    }

    return asset;
}


