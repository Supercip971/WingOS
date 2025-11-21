#pragma once

#include <stdint.h>
#include <wingos-headers/asset.h>

#include "kernel/generic/context.hpp"
#include "kernel/generic/cpu.hpp"
#include "kernel/generic/paging.hpp"
#include "kernel/generic/task.hpp"
#include "libcore/ds/vec.hpp"
#include "libcore/fmt/log.hpp"
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

    static void dump_assets(Space *space); 


    
    static core::Result<Asset *> by_handle(Space *space, uint64_t handle)
    {

        space->self->lock.lock();
        for (size_t i = 0; i < space->assets.len(); i++)
        {
            if (space->assets[i].handle == handle)
            {
                space->self->lock.release();
                return space->assets[i].asset;
            }
        }
        space->self->lock.release();

        log::log$("Asset not found in space({}) -> {} for handle {}", space->space_handle, space->uid, handle);

        log::log$("task: {}", Cpu::current()->currentTask()->uid());

        log::log$("Assets in space {}:", space->assets.len());

        for (size_t i = 0; i < space->assets.len(); i++)
        {
            log::log$("  Asset[{}]: handle={}, kind={}", i, space->assets[i].handle, assetKind2Str(space->assets[i].asset->kind));
        }
        return core::Result<Asset *>::error("asset not found");
    }

    static core::Result<AssetPtr> by_handle_ptr(Space *space, uint64_t handle)
    {
        space->self->lock.lock();

        for (size_t i = 0; i < space->assets.len(); i++)
        {
            if (space->assets[i].handle == handle)
            {
                space->self->lock.release();

                return space->assets[i];
            }
        }
        space->self->lock.release();

        return core::Result<AssetPtr>::error("asset not found");
    }
};

core::Result<AssetPtr> space_create(Space *parent, uint64_t flags, uint64_t rights);
