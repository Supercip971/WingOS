
#pragma once

#include "iol/mem_flags.h"

#include "kernel/generic/mem.hpp"
#include "kernel/generic/paging.hpp"
#include "kernel/generic/pmm.hpp"
#include "mcx/mcx.hpp"
#include "space.hpp"

Asset *_asset_create(AssetKind kind);

struct AssetMemoryCreateParams
{
    size_t size;
    size_t addr;     // the address of the memory, if 0, it will be allocated by the kernel
    bool lower_half; // if true, the memory will be allocated in priority under the 4GB limit, otherwise it will be allocated in the upper half
};

Asset *asset_create_memory(AssetMemoryCreateParams params);


struct AssetMappingCreateParams
{
    size_t start;
    size_t end;

    Asset *physical_mem;
    bool writable;
    bool executable;
};

Asset *asset_create_mapping(AssetMappingCreateParams params, VmmSpace *space = nullptr);

struct AssetTaskCreateParams
{
    kernel::CpuContextLaunch launch;
};

Asset* asset_create_task(VmmSpace* vspace, AssetTaskCreateParams params);


void asset_own(Asset *asset);

void asset_release(Asset *asset);
