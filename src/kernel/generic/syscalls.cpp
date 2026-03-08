#include "syscalls.hpp"

#include "arch/x86_64/paging.hpp"
#include "hw/mem/addr_space.hpp"
#include "libcore/fmt/impl/asset_kind.hpp"

#include "arch/x86/port.hpp"
#include "kernel/generic/asset.hpp"
#include "kernel/generic/context.hpp"
#include "kernel/generic/cpu.hpp"
#include "kernel/generic/ipc.hpp"
#include "kernel/generic/scheduler.hpp"
#include "kernel/generic/space.hpp"
#include "kernel/generic/task.hpp"
#include "libcore/core.hpp"
#include "libcore/fmt/flags.hpp"
#include "libcore/fmt/log.hpp"
#include "libcore/lock/lock.hpp"
#include "wingos-headers/asset.h"
#include "wingos-headers/syscalls.h"

core::Lock log_lock;

template <typename T>
core::Result<T *> syscall_check_ptr(kernel::Task* caller, uintptr_t ptr)
{
    if (ptr == 0)
    {
        return core::Result<T *>::error("null pointer");
    }

    if (caller == nullptr)
    {
        return core::Result<T *>::error("no current task");
    }

    if (ptr >= MMAP_IO_BASE)
    {
        return core::Result<T *>::error("invalid pointer");
    }

    try$(caller->vmm_space().verify(ptr, sizeof(T)));

    return reinterpret_cast<T *>(ptr);
}
template <typename T>
core::Result<T *> syscall_check_ptr(kernel::Task* caller, T *ptr)
{
    if (ptr == 0)
    {
        return core::Result<T *>::error("null pointer");
    }

    auto tsk = caller;
    if (tsk == nullptr)
    {
        return core::Result<T *>::error("no current task");
    }

    try$(tsk->vmm_space().verify((uintptr_t)ptr, sizeof(T)));

    return reinterpret_cast<T *>(ptr);
}

core::Result<uintptr_t> ksyscall_mem_own(kernel::Task* caller, SyscallMemOwn *mem_own)
{
    Space *space = nullptr;

    if (mem_own->target_space_handle != 0)
    {
        space = try$(
                    caller->space()->by_handle<Space>(mem_own->target_space_handle))
                    .asset;
    }
    else
    {
        space = caller->space();
    }

    if (space == nullptr)
    {
        return core::Result<uintptr_t>::error("no current space");
    }

    auto asset = try$(space->create_memory({
        .size = mem_own->size,
        .addr = mem_own->addr,
        .lower_half = false, // TODO: implement lower half allocation
    }));

    mem_own->addr = asset.asset->addr;

    mem_own->returned_handle = asset.handle;

    return (uint64_t)asset.handle;
}

core::Result<uintptr_t> ksyscall_map(kernel::Task* caller, SyscallMap *map)
{
    Space *space = nullptr;
    bool need_invalidate = false;

    if (map->target_space_handle != 0)
    {
        space = try$(
                    caller->space()->by_handle<Space>(map->target_space_handle))
                    .asset;
    }
    else
    {
        space = caller->space();
        need_invalidate = true;
    }

    if (space == nullptr)
    {
        return core::Result<uintptr_t>::error("no current space");
    }

    // Detailed logging to diagnose occasional start>=end mapping requests.
    // This should help pinpoint the caller and whether the SyscallMap struct
    // was corrupted or simply passed invalid values.
    if (map->start >= map->end)
    {
        log::err$("ksyscall_map: invalid range start>=end (start={}, end={})",
                  map->start | fmt::FMT_HEX,
                  map->end | fmt::FMT_HEX);
        return core::Result<uintptr_t>::error("invalid mapping range");
    }

    auto phys_asset = try$(space->by_handle<AssetMemory>(map->physical_mem_handle));

    auto asset = try$(space->create_mapping({
        .start = map->start,
        .end = map->end,
        .physical_mem = phys_asset,
        .writable = (map->flags & ASSET_MAPPING_FLAG_WRITE) != 0,
        .executable = (map->flags & ASSET_MAPPING_FLAG_EXECUTE) != 0,
    }));

    if (need_invalidate)
    {
        for (size_t i = map->start; i < map->end; i += arch::amd64::PAGE_SIZE)
        {
            VmmSpace::invalidate_address(VirtAddr(i));
        }
    }

    map->returned_handle = asset.handle;

    return (uint64_t)asset.handle;
}

core::Result<size_t> ksyscall_task_create(kernel::Task* caller, SyscallTaskCreate *task_create)
{
    Space *space = nullptr;
    if (task_create->target_space_handle != 0)
    {

        space = try$(
                    caller->space()->by_handle<Space>(task_create->target_space_handle))
                    .asset;
    }
    else
    {
        space = caller->space();
    }

    if (space == nullptr)
    {
        return core::Result<size_t>::error("no current space");
    }

    auto asset = try$(space->create_task((AssetTaskCreateParams){
        .launch = {
            .entry = (void *)task_create->launch,
            .stack_ptr = nullptr,
            .kernel_stack_ptr = nullptr,
            .args = {
                task_create->args[0],
                task_create->args[1],
                task_create->args[2],
                task_create->args[3],
            },
            .user = true,
        },
    }));

    task_create->returned_handle = asset.handle;

    return (uint64_t)asset.handle;
}

core::Result<size_t> ksyscall_space_create(kernel::Task* caller, SyscallSpaceCreate *args)
{
    Space *space = nullptr;
    if (args->parent_space_handle != 0)
    {
        space = try$(
                    caller->space()->by_handle<Space>(args->parent_space_handle))
                    .asset;
    }
    else
    {
        space = caller->space();
    }

    if (space == nullptr)
    {
        return core::Result<size_t>::error("no current space");
    }

    auto asset = try$(space->create_space(args->flags, args->rights));

    args->returned_handle = asset.handle;
    return (uint64_t)asset.handle;
}
core::Result<size_t> ksyscall_mem_release(kernel::Task* caller, SyscallAssetRelease *release)
{
    auto space = caller->space();
    if (space == nullptr)
    {
        return core::Result<size_t>::error("no current space");
    }

    // With the AssetRef refactor, release by explicit handle (preferred) rather than scanning raw pointers.
    // The userspace ABI already provides `asset_handle` for release.

    auto virt_mem_asset = try$(space->by_fn<AssetMapping>([&](AssetRef<AssetMapping> const &a)
                                                          {
        if(a.asset->start == (uintptr_t)release->addr && a.asset->end ==(uintptr_t) ( release->end)) {
            return true;
        }
        return false; }));

    auto mem_asset = virt_mem_asset.asset->physical_mem;

    if (mem_asset.asset == nullptr)
    {
        return core::Result<size_t>::error("memory asset not found for given address range");
    }

    space->asset_release(virt_mem_asset);
    space->asset_release(mem_asset);
    return 0ul;
}
core::Result<size_t> ksyscall_asset_release(kernel::Task* caller, SyscallAssetRelease *release)
{
    if (release->asset_handle == 0 && release->addr != nullptr)
    {
        return ksyscall_mem_release(caller, release);
    }

    Space *space = nullptr;
    if (release->space_handle != 0)
    {

        space = try$(
                    caller->space()->by_handle<Space>(release->space_handle))
                    .asset;
    }
    else
    {
        space = caller->space();
    }

    if (space == nullptr)
    {
        return core::Result<size_t>::error("no current space");
    }

    auto asset = try$(space->by_handle<Asset>(release->asset_handle));
    space->asset_release(asset);

    return 0ul;
}

core::Result<size_t> ksyscall_task_launch(kernel::Task* caller, SyscallTaskLaunch *task_launch)
{

    Space *space = nullptr;
    if (task_launch->target_space_handle != 0)
    {

        space = try$(
                    caller->space()->by_handle<Space>(task_launch->target_space_handle))
                    .asset;
    }
    else
    {
        space = caller->space();
    }

    if (space == nullptr)
    {
        return core::Result<size_t>::error("no current space");
    }

    auto task_asset = try$(space->by_handle<AssetTask>(task_launch->task_handle));
    auto task = task_asset.asset->task;

    if (task == nullptr)
    {
        return core::Result<size_t>::error("task asset has no task");
    }

    try$(kernel::task_run(task->uid()));

    return 0ul;
}

core::Result<size_t> ksyscall_asset_move(kernel::Task* caller, SyscallAssetMove *asset_move_args)
{
    Space *from_space = nullptr;
    Space *to_space = nullptr;

    if (asset_move_args->from_space_handle != 0)
    {

        from_space = try$(
                         caller->space()->by_handle<Space>(asset_move_args->from_space_handle))
                         .asset;
    }
    else
    {
        from_space = caller->space();
    }

    if (asset_move_args->to_space_handle != 0)
    {
        to_space = try$(
                       caller->space()->by_handle<Space>(asset_move_args->to_space_handle))
                       .asset;
    }
    else
    {
        to_space = caller->space();
    }

    if (from_space == nullptr || to_space == nullptr)
    {
        return core::Result<size_t>::error("no current space");
    }

    auto asset = try$(from_space->by_handle<Asset>(asset_move_args->asset_handle));

    auto moved_asset = try$(Space::asset_move(from_space, to_space, asset));

    asset_move_args->returned_handle_in_space = moved_asset.handle;

    return (uint64_t)moved_asset.handle;
}

core::Result<size_t> ksyscall_create_server(kernel::Task* caller, SyscallIpcCreateServer *create)
{
    Space *space = nullptr;
    if (create->space_handle != 0)
    {
        space = try$(
                    caller->space()->by_handle<Space>(create->space_handle))
                    .asset;
    }
    else
    {
        space = caller->space();
    }

    if (space == nullptr)
    {
        return core::Result<size_t>::error("no current space");
    }

    auto asset = try$(space->create_ipc_server({
        .is_root = create->is_root,
    }));

    // Userspace expects the server handle as an "addr" out-param.
    create->returned_addr = (uintptr_t)asset.asset->server->handle;
    create->returned_handle = asset.handle;

    return (uint64_t)asset.handle;
}
core::Result<size_t> ksyscall_create_pipe_connection(kernel::Task* caller, SyscallIpcConnect *create)
{

    Space *space_sender = nullptr;
    Space *space_receiver = nullptr;

    if (create->sender_space_handle != 0)
    {
        space_sender = try$(
                           caller->space()->by_handle<Space>(create->sender_space_handle))
                           .asset;
    }
    else
    {
        space_sender = caller->space();
    }
    if (create->receiver_space_handle != 0)
    {

        space_receiver = try$(
                             caller->space()->by_handle<Space>(create->receiver_space_handle))
                             .asset;
    }
    else
    {
        space_receiver = caller->space();
    }

    if (space_sender == nullptr || space_receiver == nullptr)
    {
        return core::Result<size_t>::error("no current space");
    }

    auto assetref = try$(Space::create_ipc_connections(
        space_sender,
        space_receiver,
        (AssetIpcConnectionPipeCreateParams){
            .flags = create->flags,
        }));

    create->returned_handle_sender = assetref.sender_connection.handle;
    create->returned_handle_receiver = assetref.receiver_connection.handle;

    return {0ul};
}

core::Result<size_t> ksyscall_create_connection(kernel::Task* caller, SyscallIpcConnect *create)
{

    if (create->flags & IPC_CONNECTION_FLAG_PIPE)
    {
        return ksyscall_create_pipe_connection(caller, create);
    }

    Space *space = nullptr;
    if (create->sender_space_handle != 0)
    {

        space = try$(
                    caller->space()->by_handle<Space>(create->sender_space_handle))
                    .asset;
    }
    else
    {
        space = caller->space();
    }

    if (space == nullptr)
    {
        return core::Result<size_t>::error("no current space");
    }

    auto asset = try$(space->create_ipc_connection((AssetIpcConnectionCreateParams){
        .server_handle = create->server_handle,
        .flags = create->flags,
    }));

    create->returned_handle_sender = asset.handle;

    return (uint64_t)asset.handle;
}

core::Result<size_t> ksyscall_send(kernel::Task* caller, SyscallIpcSend *send)
{
    Space *space = nullptr;
    if (send->space_handle != 0)
    {

        space = try$(
                    caller->space()->by_handle<Space>(send->space_handle))
                    .asset;
    }
    else
    {
        space = caller->space();
    }

    if (space == nullptr)
    {
        return core::Result<size_t>::error("no current space");
    }

    auto connection = try$(space->by_handle<AssetConnection>(send->connection_handle));

    if (!connection.asset->accepted)
    {
        return core::Result<size_t>::error("SEND: connection is not accepted");
    }

    auto res = try$(server_send_message(connection, try$(syscall_check_ptr(caller, send->message)), send->expect_reply));

    send->returned_msg_handle = res;
    return (size_t)res;
}

core::Result<size_t> ksyscall_server_receive(kernel::Task* caller, SyscallIpcServerReceive *receive)
{
    Space *space = nullptr;
    if (receive->space_handle != 0)
    {
        space = try$(
                    caller->space()->by_handle<Space>(receive->space_handle))
                    .asset;
    }
    else
    {
        space = caller->space();
    }

    if (space == nullptr)
    {
        return core::Result<size_t>::error("no current space");
    }

    auto connection = (space->by_handle<AssetConnection>(receive->connection_handle)).unwrap();

    if (!connection.asset->accepted)
    {
        log::err$("for ipc connection: ({}): {}", space->uid, receive->connection_handle);

        log::err$("for server: {}", receive->server_handle);
        log::err$("in space: {}", space->uid);
        log::log$("connection is not accepted");
        return core::Result<size_t>::error("connection is not accepted");
    }

    // DEBUG: Verify the connection belongs to the server we're receiving from
    // This catches potential bugs where a connection is being accessed by the wrong server
    if (receive->server_handle != 0)
    {
        auto server_asset_res = space->by_handle<AssetServer>(receive->server_handle);
        if (!server_asset_res.is_error())
        {
            auto server_asset = server_asset_res.unwrap();
            auto kernel_server = server_asset.asset->server;
            if (kernel_server != nullptr)
            {
                if (connection.asset->server_handle != kernel_server->handle)
                {
                    log::err$("[IPC-BUG] Server mismatch detected!");
                    log::err$("  Connection thinks it belongs to server: {}", connection.asset->server_handle);
                    log::err$("  But receive is being called from server: {}", kernel_server->handle);
                    log::err$("  Connection handle: {}, Server asset handle: {}", receive->connection_handle, receive->server_handle);
                    log::err$("  Client space: {}, Server space: {}", connection.asset->client_space_handle, connection.asset->server_space_handle);
                    return core::Result<size_t>::error("connection belongs to different server");
                }
            }
        }
    }

    auto res = (server_receive_message(connection));

    if (res.is_error())
    {
        return core::Result<size_t>(res.error());
    }

    auto received_message = res.unwrap();

    if (received_message.is_null)
    {
        receive->returned_msg_handle = 0;
        receive->contain_response = false;
        return 0ul;
    }

    if (received_message.is_disconnect)
    {
        receive->returned_msg_handle = 0;
        receive->contain_response = false;
        receive->is_disconnect = true;
        return 0ul;
    }

    receive->returned_msg_handle = received_message.uid;
    *try$(syscall_check_ptr(caller, receive->returned_message)) = received_message.message_sended.to_server();
    receive->contain_response = true;

    return (size_t)received_message.uid;
}

core::Result<size_t> ksyscall_client_receive_reply(kernel::Task* caller, SyscallIpcClientReceiveReply *receive)
{
    Space *space = nullptr;
    if (receive->space_handle != 0)
    {

        space = try$(caller->space()->by_handle<Space>(receive->space_handle)).asset;
    }
    else
    {
        space = caller->space();
    }

    if (space == nullptr)
    {
        return core::Result<size_t>::error("no current space");
    }

    auto r_connection = space->by_handle<AssetConnection>(receive->connection_handle);

    if (r_connection.is_error())
    {
        log::log$("in space({}), handle {}", receive->space_handle, receive->connection_handle);
        log::err$("Connection not found: {}", r_connection.error());
        return core::Result<size_t>::error("CLIENT RECEIVE: connection not found");
    }
    auto connection = r_connection.unwrap();

    if (!connection.asset->accepted)
    {

        log::err$("for server: {}", connection.asset->server_handle);
        log::err$("in space: (client) {}", connection.asset->client_space_handle);
        log::err$("in space: (server) {}", connection.asset->server_space_handle);

        log::log$("connection is not accepted");
        return core::Result<size_t>::error("connection is not accepted");
    }

    auto res = client_receive_response(connection, receive->message);

    if (res.is_error())
    {
        return core::Result<size_t>(res.error());
    }

    auto received_message = res.unwrap();

    if (received_message.is_null)
    {

        receive->returned_message = {};
        receive->contain_response = false;
        return 0ul;
    }

    // receive->returned_msg_handle = received_message.uid;

    *try$(syscall_check_ptr(caller, receive->returned_message)) = received_message.message_responded.to_client();
    receive->contain_response = true;

    return (size_t)received_message.uid;
}

core::Result<size_t> ksyscall_ipc_call(kernel::Task* caller, SyscallIpcCall *call)
{
    Space *space = nullptr;
    if (call->space_handle != 0)
    {

        space = try$(
                    caller->space()->by_handle<Space>(call->space_handle))
                    .asset;
    }
    else
    {
        space = caller->space();
    }

    if (space == nullptr)
    {
        return core::Result<size_t>::error("no current space");
    }

    auto connection = try$(space->by_handle<AssetConnection>(call->connection_handle));

    if (!connection.asset->accepted)
    {
        return core::Result<size_t>::error("connection is not accepted");
    }

    auto res = call_server_and_wait(connection, call->message);

    if (res.is_error())
    {
        return core::Result<size_t>(res.error());
    }

    auto received_message = (res.take());

    *try$(syscall_check_ptr(caller ,call->returned_message)) = core::move(received_message);
    call->has_reply = true;
    // call->returned_msg_handle = received_message.uid;

    return 0ul;
}

core::Result<size_t> ksyscall_ipc_accept(kernel::Task* caller, SyscallIpcAccept *accept)
{
    Space *space = nullptr;
    if (accept->space_handle != 0)
    {
        space = try$(
                    caller->space()->by_handle<Space>(accept->space_handle))
                    .asset;
    }
    else
    {
        space = caller->space();
    }

    if (space == nullptr)
    {
        return core::Result<size_t>::error("no current space");
    }

    auto server = try$(space->by_handle<AssetServer>(accept->server_handle));

    auto ipc_server = server.asset->server;
    if (ipc_server == nullptr)
    {
        return core::Result<size_t>::error("ACCEPT: server has no ipc object");
    }

    auto res = server_accept_connection(ipc_server);

    if (res.is_error())
    {
        accept->accepted_connection = false;
        return 0ul;
    }

    auto connection = res.unwrap();

    if (connection.asset == nullptr)
    {
        accept->accepted_connection = false;
        return 0ul;
    }
    // space->dump_assets();

    accept->connection_handle = connection.handle;
    accept->accepted_connection = true;

    return (size_t)connection.handle;
}

core::Result<size_t> ksyscall_ipc_server_reply(kernel::Task* caller, SyscallIpcReply *reply)
{
    Space *space = nullptr;
    if (reply->space_handle != 0)
    {
        space = try$(
                    caller->space()->by_handle<Space>(reply->space_handle))
                    .asset;
    }
    else
    {
        space = caller->space();
    }

    if (space == nullptr)
    {
        return core::Result<size_t>::error("no current space");
    }

    auto msg = try$(syscall_check_ptr(caller, reply->message));

    auto connection = try$(space->by_handle<AssetConnection>(reply->connection_handle));

    if (!connection.asset->accepted)
    {
        return core::Result<size_t>::error("connection is not accepted");
    }

    auto res = server_reply_message(connection, reply->message_handle, msg);

    if (res.is_error())
    {
        return core::Result<size_t>(res.error());
    }

    return 0ul;
}

core::Result<size_t> ksyscall_ipc_status(kernel::Task* caller, SyscallIpcStatus *status)
{
    Space *space = nullptr;

    status->returned_is_accepted = false;
    if (status->space_handle != 0)
    {
        space = try$(
                    caller->space()->by_handle<Space>(status->space_handle))
                    .asset;
        log::log$("using custom space handle: {}", status->space_handle);
    }
    else
    {
        space = caller->space();
    }

    if (space == nullptr)
    {
        return core::Result<size_t>::error("no current space");
    }

    auto connection = try$(space->by_handle<AssetConnection>(status->connection_handle));

    connection.asset->lock.lock();

    if (!connection.asset->accepted)
    {
        status->returned_is_accepted = false;
    }
    else
    {
        status->returned_is_accepted = true;
    }

    connection.asset->lock.release();

    return 0ul;
}

core::Result<size_t> ksyscall_ipc_asset_info(kernel::Task* caller, SyscallAssetInfo *info)
{
    Space *space = nullptr;
    if (info->space_handle != 0)
    {
        space = try$(
                    caller->space()->by_handle<Space>(info->space_handle))
                    .asset;
    }
    else
    {
        space = caller->space();
    }

    if (space == nullptr)
    {
        return core::Result<size_t>::error("no current space");
    }

    auto asset = try$(space->by_handle<Asset>(info->asset_handle));

    info->returned_kind = asset.asset->kind;

    switch (asset.asset->kind)
    {
    case OBJECT_KIND_MEMORY:
    {
        auto mem = asset.asset->casted<AssetMemory>();
        info->returned_info.memory.addr = mem->addr;
        info->returned_info.memory.size = mem->size;
        break;
    }
    case OBJECT_KIND_MAPPING:
    {
        auto map = asset.asset->casted<AssetMapping>();
        info->returned_info.mapping.start = map->start;
        info->returned_info.mapping.end = map->end;
        info->returned_info.mapping.writable = map->writable;
        info->returned_info.mapping.executable = map->executable;
        break;
    }
    default:
        log::warn$("Asset info for kind {} not implemented", asset.asset->kind);
        break;
    }

    return 0ul;
}

core::Result<size_t> ksyscall_ipc_x86_port(kernel::Task* caller, SyscallIpcX86Port *port)
{
    Space *space = nullptr;
    if (port->space_handle != 0)
    {
        space = try$(
                    caller->space()->by_handle<Space>(port->space_handle))
                    .asset;
    }
    else
    {
        space = caller->space();
    }

    if (space == nullptr)
    {
        return core::Result<size_t>::error("no current space");
    }

    // TODO: do right permissions check

    if (port->read)
    {
        switch (port->size)
        {
        case 1:
            port->returned_value = arch::x86::in8(port->port);
            break;
        case 2:
            port->returned_value = arch::x86::in16(port->port);
            break;
        case 4:
            port->returned_value = arch::x86::in32(port->port);
            break;
        default:
            log::err$("Invalid port size: {}", port->size);
            log::err$("Port: {}", port->port);
            return core::Result<size_t>::error("invalid size");
        }
    }
    else
    {
        switch (port->size)
        {
        case 1:
            arch::x86::out8(port->port, (uint8_t)port->data);
            break;
        case 2:
            arch::x86::out16(port->port, (uint16_t)port->data);
            break;
        case 4:
            arch::x86::out32(port->port, (uint32_t)port->data);
            break;
        default:
            log::err$("Invalid port size: {}", port->size);
            log::err$("Port: {}", port->port);
            log::err$("Data: {}", port->data);
            return core::Result<size_t>::error("invalid size");
        }
    }

    return 0ul;
}

core::Lock _sys_log_lock = {};
core::Result<size_t> syscall_handle(SyscallInterface syscall, kernel::Task* caller)
{
    switch (syscall.id)
    {
    case SYSCALL_DEBUG_LOG_ID:
    {
        log::log_lock();
        auto debug = syscall_debug_decode(syscall);
        log::log("{}", caller->uid());
        log::log("{}", debug.message);
        log::log_release();

        return 0ul;
    }
    case SYSCALL_PHYSICAL_MEM_OWN_ID:
    {
        SyscallMemOwn *mem_own = try$(syscall_check_ptr<SyscallMemOwn>(caller, syscall.arg1));

        return ksyscall_mem_own(caller, mem_own);
    }
    case SYSCALL_MAPPING_CREATE_ID:
    {
        SyscallMap *map = try$(syscall_check_ptr<SyscallMap>(caller, syscall.arg1));

        return ksyscall_map(caller, map);
    }
    case SYSCALL_TASK_CREATE_ID:
    {
        SyscallTaskCreate *task_create = try$(syscall_check_ptr<SyscallTaskCreate>(caller, syscall.arg1));
        return ksyscall_task_create(caller, task_create);
    }
    case SYSCALL_SPACE_CREATE_ID:
    {
        SyscallSpaceCreate *space_create = try$(syscall_check_ptr<SyscallSpaceCreate>(caller, syscall.arg1));
        return ksyscall_space_create(caller, space_create);
    }
    case SYSCALL_ASSET_RELEASE_ID:
    {
        SyscallAssetRelease *asset_release = try$(syscall_check_ptr<SyscallAssetRelease>(caller, syscall.arg1));
        return ksyscall_asset_release(caller, asset_release);
    }
    case SYSCALL_TASK_LAUNCH_ID:
    {
        SyscallTaskLaunch *task_launch = try$(syscall_check_ptr<SyscallTaskLaunch>(caller, syscall.arg1));
        return ksyscall_task_launch(caller, task_launch);
    }
    case SYSCALL_ASSET_MOVE:
    {
        SyscallAssetMove *asset_move = try$(syscall_check_ptr<SyscallAssetMove>(caller, syscall.arg1));
        return ksyscall_asset_move(caller, asset_move);
    }
    case SYSCALL_IPC_CREATE_SERVER_ID:
    {
        SyscallIpcCreateServer *create = try$(syscall_check_ptr<SyscallIpcCreateServer>(caller, syscall.arg1));
        return ksyscall_create_server(caller, create);
    }
    case SYSCALL_IPC_CONNECT_ID:
    {
        SyscallIpcConnect *create = try$(syscall_check_ptr<SyscallIpcConnect>(caller, syscall.arg1));
        return ksyscall_create_connection(caller, create);
    }
    case SYSCALL_IPC_SEND_ID:
    {
        SyscallIpcSend *send = try$(syscall_check_ptr<SyscallIpcSend>(caller, syscall.arg1));
        return ksyscall_send(caller, send);
    }
    case SYSCALL_IPC_SERVER_RECEIVE_ID:
    {
        SyscallIpcServerReceive *receive = try$(syscall_check_ptr<SyscallIpcServerReceive>(caller, syscall.arg1));
        return ksyscall_server_receive(caller, receive);
    }
    case SYSCALL_IPC_CLIENT_RECEIVE_REPLY_ID:
    {
        SyscallIpcClientReceiveReply *receive = try$(syscall_check_ptr<SyscallIpcClientReceiveReply>(caller, syscall.arg1));
        return ksyscall_client_receive_reply(caller, receive);
    }
    case SYSCALL_IPC_CALL_ID:
    {
        SyscallIpcCall *call = try$(syscall_check_ptr<SyscallIpcCall>(caller, syscall.arg1));
        return ksyscall_ipc_call(caller, call);
    }
    case SYSCALL_IPC_ACCEPT_ID:
    {
        SyscallIpcAccept *accept = try$(syscall_check_ptr<SyscallIpcAccept>(caller, syscall.arg1));
        return ksyscall_ipc_accept(caller, accept);
    }
    case SYSCALL_IPC_REPLY_ID:
    {
        SyscallIpcReply *reply = try$(syscall_check_ptr<SyscallIpcReply>(caller, syscall.arg1));
        return ksyscall_ipc_server_reply(caller, reply);
    }
    case SYSCALL_IPC_STATUS_ID:
    {
        SyscallIpcStatus *status = try$(syscall_check_ptr<SyscallIpcStatus>(caller, syscall.arg1));
        return ksyscall_ipc_status(caller, status);
    }
    case SYSCALL_ASSET_INFO_ID:
    {
        SyscallAssetInfo *info = try$(syscall_check_ptr<SyscallAssetInfo>(caller, syscall.arg1));
        return ksyscall_ipc_asset_info(caller, info);
    }
    case SYSCALL_IPC_X86_PORT:
    {
        SyscallIpcX86Port *port = try$(syscall_check_ptr<SyscallIpcX86Port>(caller, syscall.arg1));
        return ksyscall_ipc_x86_port(caller, port);
    }

    default:
        return {"Unknown syscall ID"};
    }
}
