#include "ipc.hpp"
#include <cstdint>

#include "hw/mem/addr_space.hpp"
#include "kernel/generic/ipc_registry.hpp"
#include <arch/x86_64/barrier.hpp>

#include "kernel/generic/paging.hpp"
#include "kernel/generic/scheduler.hpp"
#include "kernel/generic/space.hpp"
#include "libcore/fmt/flags.hpp"
#include "libcore/type-utils.hpp"
#include "wingos-headers/ipc.h"

kernel::IpcEndpoint *kernel::create_ipc_endpoint()
{
    return new kernel::IpcEndpoint();
}

void dump_message(IpcMessage const &msg)
{
    (void)msg;
    // fmt::log$(" msg.port = {}", msg.port);
    // fmt::log$(" msg.len = {}", msg.len);
    // fmt::log$(" msg.is_onull = {}", msg.is_null);
    // for (size_t i = 0; i < IpcMessageArguments::DataCount; i++)
    // {
    //     fmt::log$(" msg.arguments[{}] = {}", i, msg.arguments.data[i].data);
    // }
}

template <bool disableCheck = false>
static fc::Result<void> update_inplace_message(IpcMessageArguments *message, AssetRef<Space, disableCheck> &target_space, AssetRef<Space> &source_space)
{
    for (size_t i = 0; i < IpcMessageArguments::DataCount; i++)
    {
        if (message->data[i].is_asset)
        {
            auto asset_handle = message->data[i].asset_handle;

            auto asset_ptr_res = source_space.asset->by_handle(asset_handle);
            if (asset_ptr_res.is_error())
            {
                return ("asset not found in client space");
            }

            auto asset_ptr = asset_ptr_res.unwrap();

            if (!message->data[i].copy_asset)
            {
                message->data[i].asset_handle = try$(Space::asset_move(source_space.asset, target_space.asset, asset_ptr)).handle;
            }
            else
            {
                message->data[i].asset_handle = try$(Space::asset_copy(target_space.asset, asset_ptr)).handle;
            }
        }
    }
    return {};
}

fc::Result<void> kernel::ipc_receive_async(AssetRef<Space> &space, AssetRef<IpcEndpoint> &endpoint, IpcMessage *target, uint64_t *ret_task_handle)
{
    *ret_task_handle = 0;
    endpoint.lock();

    if (!endpoint->has_message())
    {
        endpoint.unlock();
        return {};
    }

    if (endpoint->last_async_msg_tick < endpoint->last_sync_msg_tick && endpoint->async_queue.len() != 0)
    {
        auto async_entry = (endpoint->async_queue.pop());
        *target = std::move(async_entry.target_msg);
        if (endpoint->async_queue.len() != 0)
        {
            endpoint->last_async_msg_tick = endpoint->async_queue.head().added_tick;
        }
        endpoint.unlock();
        return {};
    }

    auto sync_entry = endpoint.asset->sync_queue.pop();

    if (endpoint->sync_queue.len() != 0)
    {
        endpoint->last_sync_msg_tick = endpoint->sync_queue.head().added_tick;
    }

    if (sync_entry.is_call)
    {
        auto ret_task = try$(space->create_ipc_return_task({sync_entry.callee}));
        ret_task->floating_msg = sync_entry.kernel_mem_msg;
        *ret_task_handle = ret_task.handle;
    }
    else
    {
        sync_entry.callee->sched().unblock();
    }

    *target = std::move(*sync_entry.kernel_mem_msg);

    endpoint.unlock();

    return {};
}

fc::Result<void> kernel::ipc_receive(AssetRef<Space> &space, AssetRef<AssetTask> &callee, AssetRef<IpcEndpoint> &endpoint, IpcMessage *target, uint64_t *ret_task_handle)
{
    endpoint.lock();
    endpoint->awaiting_server = callee;
    endpoint.unlock();

    *ret_task_handle = 0;

    while (!endpoint->has_message())
    {
        callee.lock();
        callee->sched().block();

        resolve_blocked_tasks();

        callee.unlock();

        block_current_task();
    }

    endpoint.lock();

    endpoint->awaiting_server.release_ref();

    if (endpoint->last_async_msg_tick < endpoint->last_sync_msg_tick && endpoint->async_queue.len() != 0)
    {
        auto async_entry = (endpoint->async_queue.pop());
        *target = std::move(async_entry.target_msg);
        if (endpoint->async_queue.len() != 0)
        {
            endpoint->last_async_msg_tick = endpoint->async_queue.head().added_tick;
        }
        fmt::log$("received async message: {}", (uintptr_t)target | fmt::FMT_HEX);
        endpoint.unlock();
        return {};
    }

    if (endpoint->sync_queue.len() == 0)
    {
        fmt::err$("ipc_receive: sync_queue is empty");
        endpoint.unlock();
        return " empty queue return from ipc_receive";
    }

    auto sync_entry = endpoint.asset->sync_queue.pop();

    if (endpoint->sync_queue.len() != 0)
    {
        endpoint->last_sync_msg_tick = endpoint->sync_queue.head().added_tick;
    }

    //    fmt::log$("received message: {} (is call?: {})", (uintptr_t)sync_entry.kernel_mem_msg | fmt::FMT_HEX, sync_entry.is_call);
    //    dump_message(*sync_entry.kernel_mem_msg);

    *target = std::move(*sync_entry.kernel_mem_msg);

    if (sync_entry.is_call)
    {
        auto ret_task = try$(space->create_ipc_return_task({sync_entry.callee}));
        *ret_task_handle = ret_task.handle;
        ret_task->floating_msg = sync_entry.kernel_mem_msg; // use a moved trashed state don't care
    }
    else
    {
        fmt::log$("releasing mutex for async message: {}", (uintptr_t)sync_entry.kernel_mem_msg | fmt::FMT_HEX);
        sync_entry.callee->sched().unblock();

        *ret_task_handle = 0;
        resolve_blocked_tasks();
    }

    endpoint.unlock();

    return {};
}

fc::Result<void> kernel::ipc_send(AssetRef<Space> &source_space, AssetRef<AssetTask> &callee, AssetRef<IpcEndpointConnection> connection, IpcMessage *msg, bool is_call)
{
    auto &endpoint = connection->connection_to;
    msg->port = connection->port;
    auto target_space = endpoint->target_message_space;

    if (update_inplace_message(&msg->arguments, target_space, source_space).is_error())
    {
        msg->arguments = {}; // clear argument to avoid sending converted space handle
        fmt::err$("failed to update inplace message");
        return "failed to update inplace message";
    }

    endpoint.lock();

    auto phys = callee->space()->vmm_space.get_phys((uintptr_t)msg);
    if (phys.is_error())
    {
        fmt::err$("failed to get phys addr of message");
        return "failed to get phys addr of message";
    }

    // now use kernel mem:
    auto kernel_mem_msg = toVirt(phys.take()).as<IpcMessage>();

    fmt::log$("sent message: (is call?: {}) ", is_call);
    fmt::log$("sent message: {}", (uintptr_t)kernel_mem_msg | fmt::FMT_HEX);
    dump_message(*kernel_mem_msg);
    IpcSyncMsgEntry sync_entry = {
        .kernel_mem_msg = kernel_mem_msg,
        .callee = callee,
        .is_call = is_call,
        .added_tick = endpoint->tick,
    };

    endpoint->sync_queue.push(sync_entry);
    endpoint->update_msg_ticks();
    fmt::log$("sync queue: head={} tail={} len={}", endpoint->sync_queue.head().added_tick, endpoint->sync_queue.back().added_tick, endpoint->sync_queue.len());
    endpoint->tick++;

    try_disable_scheduler();

    callee->sched().block();
    if (endpoint->awaiting_server.asset)
    {
        endpoint->awaiting_server->sched().unblock();
    }
    endpoint.unlock(); // can actually schedule here ! and will read the message before blocking current task

    resolve_blocked_tasks();

    reenable_scheduler();
    block_current_task();

    fmt::log$("exited from hell: {}", (int)callee->sched().mutex.mutex_value());
    // If we are here, the call responded / was acquired
    return {};
}

fc::Result<void> kernel::ipc_reply(AssetRef<Space> &space, AssetRef<IpcMessageReturnTask> &return_task, IpcMessage *target)
{
    auto return_space = AssetRef<Space>(return_task->target->space(), 0);
    if (update_inplace_message(&target->arguments, return_space, space).is_error())
    {
        return "failed to update inplace message";
    }

    *return_task->floating_msg = std::move(*target);
    return_task->target->sched().unblock();
    space->asset_release(return_task);
    resolve_blocked_tasks();

    return {};
}

fc::Result<void> kernel::ipc_send_async(AssetRef<Space> &source_space, AssetRef<IpcEndpointConnection> connection, IpcMessage *msg)
{
    auto &endpoint = connection->connection_to;

    msg->port = connection->port;
    auto target_space = endpoint->target_message_space;
    if (update_inplace_message(&msg->arguments, target_space, source_space).is_error())
    {
        msg->arguments = {}; // clear argument to avoid sending converted space handle
        return "failed to update inplace message";
    }

    endpoint.lock();
    fmt::log$("sending async message: {}", (uintptr_t)msg | fmt::FMT_HEX);
    IpcAsyncMsgEntry sync_entry = {
        .target_msg = IpcMessage::copy(*msg),
        .added_tick = endpoint->tick,
    };
    endpoint->async_queue.push(sync_entry);
    endpoint->update_msg_ticks();
    endpoint->tick++;

    try_disable_scheduler();
    if (endpoint->awaiting_server.asset)
    {
        endpoint->awaiting_server->sched().unblock();
    }

    endpoint.unlock(); // can actually schedule here ! and will read the message before blocking current task

    resolve_blocked_tasks();

    reenable_scheduler();
    return {};
}

fc::Result<AssetRef<kernel::IpcEndpointConnection>> kernel::ipc_connect(AssetRef<Space> &target_space, AssetRef<IpcEndpoint> &endpoint)
{
    return target_space->create_ipc_connection({endpoint});
}
