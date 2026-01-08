#pragma once

#include <new>
#include <stdint.h>
#include <wingos-headers/asset.h>
#include <wingos-headers/ipc.h>

#include "kernel/generic/asset_types.hpp"
#include "kernel/generic/context.hpp"
#include "kernel/generic/cpu.hpp"
#include "kernel/generic/paging.hpp"
#include "kernel/generic/task.hpp"
#include "libcore/ds/vec.hpp"
#include "libcore/fmt/log.hpp"
#include "libcore/lock/lock.hpp"
#include "libcore/type-utils.hpp"


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

struct AssetMemory : public Asset
{

    static constexpr size_t IDENT = AssetKind::OBJECT_KIND_MEMORY;
    size_t size;
    size_t addr;
    bool allocated;

    AssetMemory(size_t size_value, size_t addr_value, bool allocated_value)
        : Asset(AssetKind::OBJECT_KIND_MEMORY), size(size_value), addr(addr_value), allocated(allocated_value) {}
};

struct AssetMapping : public Asset
{
    size_t start;
    size_t end;
    AssetRef<AssetMemory> physical_mem; // the physical memory that this mapping is based on
    bool writable;
    bool executable;

    AssetMapping(size_t start_value, size_t end_value, AssetRef<AssetMemory> physical_mem_value, bool writable_value, bool executable_value)
        : Asset(AssetKind::OBJECT_KIND_MAPPING), start(start_value), end(end_value), physical_mem(physical_mem_value), writable(writable_value), executable(executable_value) {}
};

struct AssetTask : public Asset
{
    static constexpr size_t IDENT = AssetKind::OBJECT_KIND_TASK;
    kernel::Task* task;

    AssetTask(kernel::Task* task_value)
        : Asset(AssetKind::OBJECT_KIND_TASK), task(task_value) {}
};

struct KernelIpcServer;
struct AssetServer : public Asset
{
    KernelIpcServer* server;

    static constexpr size_t IDENT = AssetKind::OBJECT_KIND_IPC_SERVER;
    AssetServer(KernelIpcServer *server_value)
        : Asset(AssetKind::OBJECT_KIND_IPC_SERVER), server(server_value) {}
};


struct IpcConnection;
struct AssetConnection  : public Asset
{
    IpcConnection* connection;

    static constexpr size_t IDENT = AssetKind::OBJECT_KIND_IPC_CONNECTION;
    AssetConnection(IpcConnection* connection_value)
        : Asset(AssetKind::OBJECT_KIND_IPC_CONNECTION), connection(connection_value) {}
};

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
struct AssetIpcServerCreateParams
{
    bool is_root; // if true, the server will be created as a root server, otherwise it will be created as a child server
};

struct AssetIpcConnectionCreateParams
{
    IpcServerHandle server_handle; // the handle of the server to connect to
    uint64_t flags;                // flags for the connection
};

struct AssetIpcConnectionPipeCreateParams
{
    uint64_t flags;                // flags for the connection
};

struct AssetIpcConnectionPipeCreateResult
{
    AssetRef<> sender_connection;
    AssetRef<> receiver_connection;
};

struct Space : public Asset
{

    constexpr static size_t IDENT = AssetKind::OBJECT_KIND_SPACE;
    void dump_assets() ;
    size_t uid;
    size_t alloc_uid;
    uint64_t space_handle;
    AssetRef<Space> parent_space_handle;
   // Space *parent_space_handle; // the space that created this space
    VmmSpace vmm_space;         // the virtual memory space of this space


    Space()
        : Asset(AssetKind::OBJECT_KIND_SPACE), uid(0), alloc_uid(0), space_handle(0), parent_space_handle(), vmm_space() {}

    core::Vec<AssetRef<>> assets;

    static AssetRef<Space> create_root();


    core::Result<AssetRef<>> by_handle( uint64_t handle)
    {

        lock.lock();
        for (size_t i = 0; i < assets.len(); i++)
        {
            if (assets[i].handle == handle)
            {
                lock.release();
                return assets[i];
            }
        }

        log::log$("Asset not found in space({}) for handle {}", uid, handle);

        auto cur = Cpu::current()->currentTask();
        log::log$("task: {}", cur ? cur->uid() : (size_t)-1);

        log::log$("Assets in space {}:", assets.len());

        for (size_t i = 0; i < assets.len(); i++)
        {
            log::log$("  Asset[{}]: handle={}, kind={}", i,assets[i].handle, assetKind2Str(assets[i].asset->kind));
        }

        lock.release();

        return "asset not found";

    }
    template<typename T>
        core::Result<AssetRef<T>> by_handle(uint64_t handle)
        {
            lock.lock();
            for (size_t i = 0; i < assets.len(); i++)
            {
                if (assets[i].handle == handle)
                {
                    lock.release();

                    // Untyped lookup: allow returning any asset when caller asks for `Asset`.
                    if constexpr (core::IsSame<T, Asset>)
                    {
                        return assets[i];
                    }
                    else
                    {
                        if (assets[i].asset->kind == (AssetKind)T::IDENT)
                        {
                            return assets[i].casted<T>();
                        }
                        else
                        {
                            log::log$("expected: {}, got: {} for raw id: {}",
                                T::IDENT,
                                assets[i].asset->kind,
                                assets[i].handle);
                            return core::Result<AssetRef<T>>::error("asset kind mismatch");
                        }
                    }
                }
            }
            lock.release();

            return core::Result<AssetRef<T>>::error("asset not found");
        }
    core::Result<AssetRef<>> by_handle_ptr(uint64_t handle)
    {
        lock.lock();
        for (size_t i = 0; i < assets.len(); i++)
        {
            if (assets[i].handle == handle)
            {
                lock.release();
                return assets[i];
            }
        }
        lock.release();

        return ("asset not found");
    }


    template<typename T, typename ...Args>
    core::Result<AssetRef<T>> allocate_asset(Args&&... args)
    {
        T* res = new T(args...);
        res->lock.lock();  // Lock the asset before adding to space

        lock.lock();
        auto handle = alloc_uid;
        alloc_uid++;
        AssetRef<T> ref = AssetRef<T>(res, handle, true, true, true);

        assets.push(ref.to_untyped());
        lock.release();

        // Asset lock is still held - caller must release after initialization
        return ref;
    }


    // asset_release is defined as template below after _asset_remove

    core::Result<AssetRef<Space>> create_space(uint64_t flags, uint64_t rights);

    static core::Result<AssetRef<Space>> global_space_by_handle(uint64_t handle);


    AssetRef<> _asset_remove(uint64_t asset_handle)
    {
        lock.lock();
        for (size_t i = 0; i < assets.len(); i++)
        {
            if (assets[i].handle == asset_handle)
            {
                // pop returns the removed AssetRef, which will be destroyed
                // and call Asset::deref. This is the only deref we want.
                auto val  = assets.pop(i);
                lock.release();
                return (val);
            }
        }
        lock.release();

        __builtin_unreachable();

    }

    core::Result<AssetRef<AssetMemory>> create_memory( AssetMemoryCreateParams params);

    core::Result<AssetRef<AssetMapping>> create_mapping(AssetMappingCreateParams params);

    core::Result<AssetRef<AssetTask>> create_task(AssetTaskCreateParams params);

    core::Result<AssetRef<AssetServer>> create_ipc_server(
        AssetIpcServerCreateParams params);

    core::Result<AssetRef<AssetConnection>> create_ipc_connection(
        AssetIpcConnectionCreateParams params);

    static core::Result<AssetIpcConnectionPipeCreateResult> create_ipc_connections(
        Space* sender, Space* receiver, AssetIpcConnectionPipeCreateParams params);

    template<typename T>
    static core::Result<AssetRef<>> asset_move(
        Space* from, Space* to, AssetRef<T> const& asset)
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
        Space* first = from < to ? from : to;
        Space* second = from < to ? to : from;

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
                moved_asset.handle = to->alloc_uid++;
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

    template<typename T>
    static core::Result<AssetRef<>> asset_copy(
        Space* from, Space* to, AssetRef<T> const& asset)
    {
        if (from == nullptr || to == nullptr)
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

        auto nref = AssetRef<>(reinterpret_cast<Asset*>(asset.asset), to->alloc_uid++);
        to->assets.push(nref);

        to->lock.release();

        asset.asset->lock.release();

        return nref;
    }

    template<typename T>
    void asset_release(AssetRef<T> const& ref)
    {
        _asset_remove(ref.handle);

    }

    };

struct KernelIpcServer;
struct IpcConnection;

//core::Result<AssetRef> space_create(Space *parent, uint64_t flags, uint64_t rights);
