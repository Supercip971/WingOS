
#include "asset.hpp"


Asset * _asset_create(AssetKind kind)
{
    Asset *asset = new Asset();
    *asset = {};
    asset->kind = kind;
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

Asset *asset_create_memory(AssetMemoryCreateParams params)
{
    Asset *asset = _asset_create(OBJECT_KIND_MEMORY);
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
    return asset;
}

Asset *asset_create_mapping(AssetMappingCreateParams params, VmmSpace *space)
{
    Asset *asset = _asset_create(OBJECT_KIND_MAPPING);

    if (params.start >= params.end)
    {
        log::err$("asset_create_mapping: start must be less than end");
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

    if (!space->map({asset->mapping.start, asset->mapping.end},
                    {asset->mapping.physical_mem->memory.addr, asset->mapping.physical_mem->memory.size},
                   flags ))
    {
        log::err$("asset_create_mapping: failed to map memory");
        return nullptr;
    }

    return asset;
}

Asset* asset_create_task(VmmSpace* vspace, AssetTaskCreateParams params)
{
    Asset *asset = _asset_create(OBJECT_KIND_TASK);
    asset->task = kernel::Task::task_create().unwrap();
    if (asset->task == nullptr)
    {
        log::err$("unable to create task asset");
        return nullptr;
    }

    if (!asset->task->_initialize(params.launch, vspace))
    {
        log::err$("unable to initialize task asset");
        return nullptr;
    }

    return asset;
}


