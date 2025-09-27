#pragma once

#include <stdint.h>
#include <wingos-headers/asset.h>

#include "kernel/generic/paging.hpp"
#include "kernel/generic/task.hpp"
#include "libcore/ds/vec.hpp"
#include "libcore/lock/lock.hpp"

struct Asset;

struct AssetPtr
{
    Asset *asset;
    uint64_t handle; // the handle of the asset in the space
    bool write;
    bool read;
    bool share;
};

struct Space
{

    size_t uid;
    size_t alloc_uid;
    Asset *self;
    uint64_t space_handle;
    Space *parent_space_handle; // the space that created this space
    VmmSpace vmm_space;         // the virtual memory space of this space

    core::Vec<AssetPtr> assets;

    static core::Result<Space *> space_by_handle(Space *parent, uint64_t handle);
    static core::Result<Space *> global_space_by_handle(uint64_t handle);
};

struct KernelIpcServer;
struct IpcConnection;
struct Asset
{
    core::Lock lock;

    size_t ref_count;
    AssetKind kind;
    union
    {
        struct
        {
            size_t size;
            size_t addr;
            bool allocated;
        } memory;

        struct
        {
            size_t start;
            size_t end;
            Asset *physical_mem; // the physical memory that this mapping is based on
            bool writable;
            bool executable;
        } mapping;

        Space *space;

        kernel::Task *task;

        KernelIpcServer *ipc_server; // the IPC server that this asset is associated with

        IpcConnection *ipc_connection;
    };

    static core::Result<Asset *> by_handle(Space *space, uint64_t handle)
    {
        for (size_t i = 0; i < space->assets.len(); i++)
        {
            if (space->assets[i].handle == handle)
            {
                return space->assets[i].asset;
            }
        }
        return core::Result<Asset *>::error("asset not found");
    }

    static core::Result<AssetPtr> by_handle_ptr(Space *space, uint64_t handle)
    {
        for (size_t i = 0; i < space->assets.len(); i++)
        {
            if (space->assets[i].handle == handle)
            {
                return space->assets[i];
            }
        }
        return core::Result<AssetPtr>::error("asset not found");
    }
};

core::Result<AssetPtr> space_create(Space *parent, uint64_t flags, uint64_t rights);
