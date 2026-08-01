#pragma once

#include <atomic>
#include <stdint.h>
#include <wingos-headers/asset.h>
#include <wingos-headers/ipc.h>

#include "kernel/generic/asset_types.hpp"
#include "kernel/generic/ipc_asset.hpp"

#include "asset.hpp"
#include "kernel/generic/context.hpp"
#include "kernel/generic/cpu.hpp"
#include "kernel/generic/paging.hpp"
#include "kernel/generic/task.hpp"
#include "libcore/ds/vec.hpp"
#include "libcore/fmt/log.hpp"
#include "libcore/lock/lock.hpp"
#include "libcore/result.hpp"
#include "libcore/type/trait.hpp"
#include "libcore/unreachable.h"

/*union
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
};*/

struct AssetMemoryCreateParams
{
    size_t size;
    size_t addr;     // the address of the memory, if 0, it will be allocated by the kernel
    bool lower_half; // if true, the memory will be allocated in priority under the 4GB limit, otherwise it will be allocated in the upper half
};

struct AssetMappingCreateParams
{
    size_t start;
    size_t end;

    AssetRef<AssetMemory> physical_mem;
    bool writable;
    bool executable;
};

struct AssetTaskCreateParams
{
    kernel::CpuContextLaunch launch;
};

struct AssetIpcReturnTaskCreateParams
{
    AssetRef<AssetTask> task;
};

struct AssetIpcEndpointCreateParams
{
    bool publish;
    bool is_root;
};

struct AssetIpcConnectionCreateParams
{
    AssetRef<kernel::IpcEndpoint> endpoint;
};

struct AssetIpcConnectionPipeCreateResult
{
    AssetRef<> sender_connection;
    AssetRef<> receiver_connection;
};

struct Space : public Asset
{

    constexpr static size_t IDENT = AssetKind::OBJECT_KIND_SPACE;
    void dump_assets();
    size_t uid;
    std::atomic<size_t> alloc_uid;
    // Space *parent_space_handle; // the space that created this space
    VmmSpace vmm_space; // the virtual memory space of this space

    fc::Vec<AssetRef<>> assets;

    Space()
        : Asset(AssetKind::OBJECT_KIND_SPACE), uid(0), alloc_uid(0), vmm_space(), assets()
    {
    }

    static AssetRef<Space> create_root();

    template <typename T>
    fc::Result<AssetRef<T>> by_fn(auto fn)
    {

        lock.lock();
        for (size_t i = 0; i < assets.len(); i++)
        {
            if (assets[i].asset->kind != (AssetKind)T::IDENT)
            {
                continue;
            }

            if (fn(assets[i].casted<T>()))
            {
                auto result = assets[i]; // copy under lock (increments refcount)
                lock.release();
                return result.casted<T>();
            }
        }

        lock.release();
        return "not found";
    }

    fc::Result<AssetRef<>> by_handle(uint64_t handle)
    {

        lock.lock();
        for (size_t i = 0; i < assets.len(); i++)
        {
            if (assets[i].handle == handle)
            {
                auto result = assets[i]; // copy under lock (increments refcount)
                lock.release();
                return result;
            }
        }

        fmt::log$("Asset not found in space({}) for handle {}", uid, handle);

        auto cur = Cpu::current()->currentTask();
        fmt::log$("task: {}", cur ? cur->uid() : (size_t)-1);

        fmt::log$("Assets in space {}:", assets.len());

        for (size_t i = 0; i < assets.len(); i++)
        {
            fmt::log$("  Asset[{}]: handle={}, kind={}", i, assets[i].handle, assetKind2Str(assets[i].asset->kind));
        }

        lock.release();

        return "asset not found";
    }

    template <typename T>
    fc::Result<AssetRef<T>> by_handle(uint64_t handle)
    {
        lock.lock();
        for (size_t i = 0; i < assets.len(); i++)
        {
            if (assets[i].handle == handle)
            {
                AssetRef<> found = assets[i]; // copy under lock (increments refcount)
                lock.release();

                // Untyped lookup: allow returning any asset when caller asks for `Asset`.
                if constexpr (fc::IsSame<T, Asset>)
                {

                    return found;
                }
                else
                {

                    found.lock();
                    if (found.asset->kind == (AssetKind)T::IDENT)
                    {
                        found.unlock();

                        return found.casted<T>();
                    }
                    else
                    {

                        found.unlock();
                        fmt::err$("expected: {}, got: {} for raw id: {} (space: {})",
                                  assetKind2Str((AssetKind)T::IDENT),
                                  found.asset->kind,
                                  found.handle, this->uid);

                        return fc::Result<AssetRef<T>>::error("asset kind mismatch");
                    }
                }
            }
        }
        asm volatile("cli");

        fmt::log$("Asset not found in space({}) for handle {}", uid, handle);

        auto cur = Cpu::current()->currentTask();
        fmt::log$("task: {}", cur ? cur->uid() : (size_t)-1);

        fmt::log$("Assets in space {}:", assets.len());

        for (size_t i = 0; i < assets.len(); i++)
        {
            fmt::log$("  Asset[{}]: handle={}, kind={}", i, assets[i].handle, assetKind2Str(assets[i].asset->kind));
        }

        lock.release();

        return fc::Result<AssetRef<T>>::error("asset not found");
    }

    fc::Result<AssetRef<>> by_handle_ptr(uint64_t handle)
    {
        lock.lock();
        for (size_t i = 0; i < assets.len(); i++)
        {
            if (assets[i].handle == handle)
            {
                auto result = assets[i]; // copy under lock (increments refcount)
                lock.release();
                return result;
            }
        }
        lock.release();

        return ("asset not found");
    }

    template <typename T>
    fc::Result<AssetRef<T>> add_asset(T *res)
    {

        res->lock.lock(); // Lock the asset before adding to space

        lock.lock();

        alloc_uid++;
        size_t nhandle = alloc_uid;
        AssetRef<T> ref = AssetRef<T>(res, nhandle, true, true, true);

        assets.push(ref.to_untyped());
        lock.release();

        // Asset lock is still held - caller must release after initialization
        return ref;
    }

    template <typename T, typename... Args>
    fc::Result<AssetRef<T>> allocate_asset(Args &&...args)
    {

        T *res = new T(args...);
        res->lock.lock(); // Lock the asset before adding to space

        lock.lock();

        alloc_uid++;
        size_t nhandle = alloc_uid;
        AssetRef<T> ref = AssetRef<T>(res, nhandle, true, true, true);

        assets.push(ref.to_untyped());
        lock.release();

        // Asset lock is still held - caller must release after initialization
        return ref;
    }

    // asset_release is defined as template below after _asset_remove

    fc::Result<AssetRef<Space>> create_space(uint64_t flags, uint64_t rights);

    static fc::Result<AssetRef<Space>> global_space_by_handle(uint64_t handle);

    AssetRef<> _asset_remove(uint64_t asset_handle)
    {
        lock.lock();
        for (size_t i = 0; i < assets.len(); i++)
        {
            if (assets[i].handle == asset_handle)
            {
                // pop returns the removed AssetRef, which will be destroyed
                // and call Asset::deref. This is the only deref we want.
                auto val = assets.pop(i);
                lock.release();
                return (val);
            }
        }
        lock.release();

        unreachable$();
    }

    fc::Result<AssetRef<AssetMemory>> create_memory(AssetMemoryCreateParams params);

    fc::Result<AssetRef<AssetMapping>> create_mapping(AssetMappingCreateParams params);

    fc::Result<AssetRef<AssetTask>> create_task(AssetTaskCreateParams params);

    fc::Result<AssetRef<kernel::IpcMessageReturnTask>> create_ipc_return_task(AssetIpcReturnTaskCreateParams const &params);

    fc::Result<AssetRef<kernel::IpcEndpointConnection>> create_ipc_connection(AssetIpcConnectionCreateParams const &params);

    fc::Result<AssetRef<kernel::IpcEndpoint>> create_ipc_endpoint(AssetIpcEndpointCreateParams const &params);

    template <typename T>
    static fc::Result<AssetRef<>> asset_move(
        Space *from, Space *to, AssetRef<T> const &asset)
    {
        if (from == nullptr || to == nullptr)
        {
            return ("from or to space is null");
        }

        if (asset.asset == nullptr)
        {
            return ("asset is null");
        }

        // Lock spaces in consistent order (by address) to prevent ABBA deadlock
        Space *first = from < to ? from : to;
        Space *second = from < to ? to : from;

        first->lock.lock();
        if (first != second)
        {
            second->lock.lock();
        }

        // Check if the asset exists in the from space
        for (size_t i = 0; i < from->assets.len(); i++)
        {
            if (from->assets[i].handle == asset.handle)
            {
                // Move the asset to the new space
                auto moved_asset = from->assets.pop(i);
                to->alloc_uid++;
                moved_asset.handle = to->alloc_uid;
                to->assets.push(moved_asset);

                if (first != second)
                {
                    second->lock.release();
                }
                first->lock.release();

                return moved_asset;
            }
        }

        if (first != second)
        {
            second->lock.release();
        }
        first->lock.release();

        return ("asset not found in from space");
    }

    template <typename T>
    static fc::Result<AssetRef<>> asset_copy(
        Space *to, AssetRef<T> const &asset)
    {
        if (to == nullptr)
        {
            return "from or to space is null";
        }

        if (asset.asset == nullptr)
        {
            return "asset is null";
        }

        // Lock the asset to prevent it from being deleted during copy
        asset.asset->lock.lock();

        // Lock the destination space before modifying its assets
        to->lock.lock();

        to->alloc_uid++;
        auto nref = AssetRef<>(reinterpret_cast<Asset *>(asset.asset), to->alloc_uid);

        to->assets.push(nref);

        to->lock.release();

        asset.asset->lock.release();

        return nref;
    }

    void asset_release_by_handle(uint64_t handle)
    {
        auto v = _asset_remove(handle);
        fmt::log$("Releasing asset: {} kind: {}", handle, assetKind2Str(v.asset->kind));
        Asset::release(v.asset);
    }

    template <typename T>
    void asset_release(AssetRef<T> const &ref)
    {
        // Mark the asset as "released" (e.g. close IPC connections, unregister
        // servers) BEFORE removing it from the space's asset list.  This ensures
        // that other CPUs that still hold an AssetRef see the updated status
        // (closed / destroyed) instead of stale IPC_STILL_OPEN values —
        // fixing "incorrect status" races under SMP.
        if (ref.asset != nullptr)
        {

            Asset::release(reinterpret_cast<Asset *>(ref.asset));

            auto res = _asset_remove(ref.handle);
        }
    }
};

// fc::Result<AssetRef> space_create(Space *parent, uint64_t flags, uint64_t rights);
