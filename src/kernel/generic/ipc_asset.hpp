#pragma once

#include <climits>

#include "kernel/generic/asset_types.hpp"

#include "kernel/generic/asset.hpp"
#include "libcore/ds/ring.hpp"
#include "wingos-headers/asset.h"
#include "wingos-headers/ipc.h"

// Forward declarations to avoid pulling `space.hpp` (and its dependencies) into IPC.
struct Space;

namespace kernel
{
struct IpcMessageReturnTask : public Asset
{
    static constexpr size_t IDENT = AssetKind::OBJECT_KIND_IPC_RETURN_TASK;
    AssetRef<AssetTask> target;

    IpcMessageReturnTask(AssetRef<AssetTask> const &target_value) : Asset(AssetKind::OBJECT_KIND_IPC_RETURN_TASK), target(target_value) {}
};

struct IpcSyncMsgEntry
{
    IpcMessage *msg;
    AssetRef<AssetTask> callee;
    bool is_call;
    long added_tick;
};

struct IpcAsyncMsgEntry
{
    IpcMessage target_msg;
    uint64_t added_tick;
};

struct IpcEndpoint : public Asset
{
    static constexpr size_t IDENT = AssetKind::OBJECT_KIND_IPC_ENDPOINT;
    long tick;
    fc::Ring<IpcAsyncMsgEntry> async_queue; // only sent, no result
    long last_async_msg_tick;
    fc::Ring<IpcSyncMsgEntry> sync_queue; // sync sent + sync call
    long last_sync_msg_tick;

    AssetRef<Space, true> target_message_space;
    AssetRef<AssetTask> awaiting_server;

    long uuid;
    long last_port;

    IpcEndpoint() : Asset(AssetKind::OBJECT_KIND_IPC_ENDPOINT)
    {
    }

    void update_msg_ticks()
    {
        last_async_msg_tick = async_queue.len() != 0 ? async_queue.head().added_tick : LONG_MAX;
        last_sync_msg_tick = sync_queue.len() != 0 ? sync_queue.head().added_tick : LONG_MAX;
    }

    bool has_message()
    {
        this->lock.lock();
        bool result = async_queue.len() != 0 || sync_queue.len() != 0;
        this->lock.release();
        return result;
    }

    virtual ~IpcEndpoint() = default;
};

struct IpcEndpointConnection : public Asset
{
    static constexpr size_t IDENT = AssetKind::OBJECT_KIND_IPC_CONNECTION;
    AssetRef<IpcEndpoint> connection_to;
    uint64_t port;

    IpcEndpointConnection() : Asset(AssetKind::OBJECT_KIND_IPC_CONNECTION) {}

    virtual ~IpcEndpointConnection() = default;
};

} // namespace kernel
