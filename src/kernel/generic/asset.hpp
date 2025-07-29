
#pragma once

#include "iol/mem_flags.h"

#include "kernel/generic/mem.hpp"
#include "kernel/generic/paging.hpp"
#include "kernel/generic/pmm.hpp"
#include "mcx/mcx.hpp"
#include "space.hpp"

#include "kernel/generic/ipc.hpp"
core::Result<AssetPtr> _asset_create(Space *space, AssetKind kind);

struct AssetMemoryCreateParams
{
    size_t size;
    size_t addr;     // the address of the memory, if 0, it will be allocated by the kernel
    bool lower_half; // if true, the memory will be allocated in priority under the 4GB limit, otherwise it will be allocated in the upper half
};

core::Result<AssetPtr> asset_create_memory(Space *space, AssetMemoryCreateParams params);

struct AssetMappingCreateParams
{
    size_t start;
    size_t end;

    Asset *physical_mem;
    bool writable;
    bool executable;
};

core::Result<AssetPtr> asset_create_mapping(Space *space, AssetMappingCreateParams params);

struct AssetTaskCreateParams
{
    kernel::CpuContextLaunch launch;
};

core::Result<AssetPtr> asset_create_task(Space *vspace, AssetTaskCreateParams params);

struct AssetIpcServerCreateParams
{
    bool is_root; // if true, the server will be created as a root server, otherwise it will be created as a child server
};

core::Result<AssetPtr> asset_create_ipc_server(Space *space, AssetIpcServerCreateParams params);

struct AssetIpcConnectionCreateParams
{
    IpcServerHandle server_handle; // the handle of the server to connect to
    uint64_t flags; // flags for the connection
};

// <!> create two object: one for the connection and one for the server
core::Result<AssetPtr> asset_create_ipc_connections(Space *space, AssetIpcConnectionCreateParams params); 

core::Result<AssetPtr> asset_move(Space* from, Space* to, AssetPtr asset);
core::Result<AssetPtr> asset_copy(Space* from, Space* to, AssetPtr asset);



void asset_own(Asset *asset);

void asset_release(Space *space, Asset *asset);
