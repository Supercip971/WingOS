#include "syscalls.hpp"

#include "arch/x86_64/paging.hpp"

#include "kernel/generic/asset.hpp"
#include "kernel/generic/context.hpp"
#include "kernel/generic/cpu.hpp"
#include "kernel/generic/ipc.hpp"
#include "kernel/generic/scheduler.hpp"
#include "kernel/generic/task.hpp"
#include "libcore/fmt/log.hpp"
#include "libcore/lock/lock.hpp"
#include "wingos-headers/asset.h"
#include "wingos-headers/syscalls.h"


core::Lock log_lock;
template <typename T>
core::Result<T *> syscall_check_ptr(uintptr_t ptr)
{
    if (ptr == 0)
    {
        return core::Result<T *>::error("null pointer");
    }

    auto tsk = Cpu::current()->currentTask();
    if (tsk == nullptr)
    {
        return core::Result<T *>::error("no current task");
    }

    try$(tsk->vmm_space().verify(ptr, sizeof(T)));

    return reinterpret_cast<T *>(ptr);
}
template <typename T>
core::Result<T *> syscall_check_ptr(T* ptr)
{
    if (ptr == 0)
    {
        return core::Result<T *>::error("null pointer");
    }

    auto tsk = Cpu::current()->currentTask();
    if (tsk == nullptr)
    {
        return core::Result<T *>::error("no current task");
    }

    try$(tsk->vmm_space().verify((uintptr_t)ptr, sizeof(T)));

    return reinterpret_cast<T *>(ptr);
}

core::Result<uintptr_t> ksyscall_mem_own(SyscallMemOwn *mem_own)
{
    Space *space = nullptr;

    if (mem_own->target_space_handle != 0)
    {
        space = try$(Space::space_by_handle(
            Cpu::current()->currentTask()->space(),

            mem_own->target_space_handle));
    }
    else
    {
        space = Cpu::current()->currentTask()->space();
    }

    if (space == nullptr)
    {
        return core::Result<size_t>::error("no current space");
    }

    auto asset = try$(asset_create_memory(space, {
                                                     .size = mem_own->size,
                                                     .addr = mem_own->addr,
                                                     .lower_half = false, // TODO: implement lower half allocation
                                                 }));

    mem_own->addr = asset.asset->memory.addr;

    mem_own->returned_handle = asset.handle;

    return core::Result<size_t>::success((uint64_t)asset.handle);

    return {};
}

core::Result<uintptr_t> ksyscall_map(SyscallMap *map)
{
    Space *space = nullptr;
    bool need_invalidate = false;
    if (map->target_space_handle != 0)
    {
        space = try$(Space::space_by_handle(
            Cpu::current()->currentTask()->space(),

            map->target_space_handle));
    }
    else
    {
        space = Cpu::current()->currentTask()->space();
        need_invalidate = true;
    }

    if (space == nullptr)
    {
        return core::Result<size_t>::error("no current space");
    }

    auto asset = try$(asset_create_mapping(space, {
                                                      .start = map->start,
                                                      .end = map->end,
                                                      .physical_mem = try$(Asset::by_handle(space, map->physical_mem_handle)),
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

    return core::Result<size_t>::success((uint64_t)asset.handle);
}

core::Result<size_t> ksyscall_task_create(SyscallTaskCreate *task_create)
{
    Space *space = nullptr;
    if (task_create->target_space_handle != 0)
    {
        space = try$(Space::space_by_handle(
            Cpu::current()->currentTask()->space(),

            task_create->target_space_handle));
    }
    else
    {
        space = Cpu::current()->currentTask()->space();
    }

    if (space == nullptr)
    {
        return core::Result<size_t>::error("no current space");
    }

    auto asset = try$(asset_create_task(space, (AssetTaskCreateParams){
                                                   .launch = {
                                                       .entry = (void *)task_create->launch,
                                                       .user = true,
                                                   },
                                               }));

    task_create->returned_handle = asset.handle;
    return core::Result<size_t>::success((uint64_t)asset.handle);
}

core::Result<size_t> ksyscall_space_create(SyscallSpaceCreate *args)
{
    Space *space = nullptr;
    if (args->parent_space_handle != 0)
    {
        space = try$(Space::space_by_handle(
            Cpu::current()->currentTask()->space(),

            args->parent_space_handle));
    }
    else
    {
        space = Cpu::current()->currentTask()->space();
    }

    if (space == nullptr)
    {
        return core::Result<size_t>::error("no current space");
    }

    auto asset = try$(space_create(space, args->flags, args->rights));

    args->returned_handle = asset.handle;
    return core::Result<size_t>::success((uint64_t)asset.handle);
}
core::Result<size_t> ksyscall_mem_release(SyscallAssetRelease *release)
{

    auto space = Cpu::current()->currentTask()->space();
    if (space == nullptr)
    {
        return core::Result<size_t>::error("no current space");
    }

    Asset *phys_mem = nullptr;
    Asset *virt_mem = nullptr;
    for (size_t i = 0; i < space->assets.len(); i++)
    {
        if (space->assets[i].asset->kind == OBJECT_KIND_MAPPING && space->assets[i].asset->mapping.start == (uintptr_t)release->addr)
        {

            virt_mem = space->assets[i].asset;
            phys_mem = space->assets[i].asset->mapping.physical_mem;
            break;
        }
    }
    if (phys_mem == nullptr || virt_mem == nullptr)
    {
        return core::Result<size_t>::error("no memory mapping found for address");
    }

    if (phys_mem->kind != OBJECT_KIND_MEMORY)
    {
        return core::Result<size_t>::error("physical memory asset is not a memory asset");
    }

    if (virt_mem->kind != OBJECT_KIND_MAPPING)
    {
        return core::Result<size_t>::error("virtual memory asset is not a mapping asset");
    }

    asset_release(space, phys_mem);
    asset_release(space, virt_mem);

    return core::Result<size_t>::success(0);
}
core::Result<size_t> ksyscall_asset_release(SyscallAssetRelease *release)
{

    if (release->asset_handle == 0 && release->addr != nullptr)
    {
        return ksyscall_mem_release(release);
    }

    Space *space = nullptr;
    if (release->space_handle != 0)
    {
        space = try$(Space::space_by_handle(
            Cpu::current()->currentTask()->space(),

            release->space_handle));
    }
    else
    {
        space = Cpu::current()->currentTask()->space();
    }

    if (space == nullptr)
    {
        return core::Result<size_t>::error("no current space");
    }

    auto asset = try$(Asset::by_handle(space, release->asset_handle));

    asset_release(space, asset);

    return core::Result<size_t>::success(0);
}

core::Result<size_t> ksyscall_task_launch(SyscallTaskLaunch *task_launch)
{
    Space *space = nullptr;
    if (task_launch->target_space_handle != 0)
    {
        space = try$(Space::space_by_handle(
            Cpu::current()->currentTask()->space(),
            task_launch->target_space_handle));
    }
    else
    {
        space = Cpu::current()->currentTask()->space();
    }

    if (space == nullptr)
    {
        return core::Result<size_t>::error("no current space");
    }

    auto task_asset = try$(Asset::by_handle(space, task_launch->task_handle));
    if (task_asset->kind != OBJECT_KIND_TASK)
    {
        return core::Result<size_t>::error("task asset is not a task");
    }
    auto task = task_asset->task;
    if (task == nullptr)
    {
        return core::Result<size_t>::error("task asset has no task");
    }

    try$(kernel::task_run(task->uid()));

    return core::Result<size_t>::success(0);
}

core::Result<size_t> ksyscall_asset_move(SyscallAssetMove *asset_move_args)
{
    Space *from_space = nullptr;
    Space *to_space = nullptr;

    if (asset_move_args->from_space_handle != 0)
    {
        from_space = try$(Space::space_by_handle(
            Cpu::current()->currentTask()->space(),
            asset_move_args->from_space_handle));
    }
    else
    {
        from_space = Cpu::current()->currentTask()->space();
    }

    if (asset_move_args->to_space_handle != 0)
    {
        to_space = try$(Space::space_by_handle(
            Cpu::current()->currentTask()->space(),
            asset_move_args->to_space_handle));
    }
    else
    {
        to_space = Cpu::current()->currentTask()->space();
    }

    if (from_space == nullptr || to_space == nullptr)
    {
        return core::Result<size_t>::error("no current space");
    }

    auto asset = try$(Asset::by_handle_ptr(from_space, asset_move_args->asset_handle));

    auto moved_asset = try$(asset_move(from_space, to_space, asset));

    asset_move_args->returned_handle_in_space = moved_asset.handle;

    return core::Result<size_t>::success((uint64_t)moved_asset.handle);
}

core::Result<size_t> ksyscall_create_server(SyscallIpcCreateServer *create)
{
    Space *space = nullptr;
    if (create->space_handle != 0)
    {
        space = try$(Space::space_by_handle(
            Cpu::current()->currentTask()->space(),
            create->space_handle));
    }
    else
    {
        space = Cpu::current()->currentTask()->space();
    }

    if (space == nullptr)
    {
        return core::Result<size_t>::error("no current space");
    }

    auto asset = try$(asset_create_ipc_server(space, {
        .is_root = create->is_root,
    }));

    create->returned_addr = asset.asset->ipc_server->handle;
    create->returned_handle = asset.handle;

    return core::Result<size_t>::success((uint64_t)asset.handle);
}

core::Result<size_t> ksyscall_create_connection(SyscallIpcConnect *create)
{
    Space *space = nullptr;
    if (create->space_handle != 0)
    {
        space = try$(Space::space_by_handle(
            Cpu::current()->currentTask()->space(),
            create->space_handle));
    }
    else
    {
        space = Cpu::current()->currentTask()->space();
    }

    if (space == nullptr)
    {
        return core::Result<size_t>::error("no current space");
    }

    auto asset = try$(asset_create_ipc_connections(space, (AssetIpcConnectionCreateParams){
        .server_handle = create->server_handle,
        .flags = create->flags,
    }));

    create->returned_handle = asset.handle;

    return core::Result<size_t>::success((uint64_t)asset.handle);
}

core::Result<size_t> ksyscall_send(SyscallIpcSend* send) 
{
    Space *space = nullptr;
    if (send->space_handle != 0)
    {
        space = try$(Space::space_by_handle(
            Cpu::current()->currentTask()->space(),
            send->space_handle));
    }
    else
    {
        space = Cpu::current()->currentTask()->space();
    }

    if (space == nullptr)
    {
        return core::Result<size_t>::error("no current space");
    }

    auto connection = try$(Asset::by_handle(space, send->connection_handle));

    if (connection->kind != OBJECT_KIND_IPC_CONNECTION)
    {
        return core::Result<size_t>::error("connection is not an IPC connection");
    }

    auto ipc_connection = connection->ipc_connection;

    if(!ipc_connection->accepted)
    {
        return core::Result<size_t>::error("connection is not accepted");
    }
    


    auto res = try$(server_send_message(ipc_connection, try$( syscall_check_ptr( send->message)), send->expect_reply));

    send->returned_msg_handle = res;
    return core::Result<size_t>::success((size_t)res);
}

core::Result<size_t> ksyscall_server_receive(SyscallIpcServerReceive* receive)
{
    Space *space = nullptr;
    if (receive->space_handle != 0)
    {
        space = try$(Space::space_by_handle(
            Cpu::current()->currentTask()->space(),
            receive->space_handle));
    }
    else
    {
        space = Cpu::current()->currentTask()->space();
    }

    if (space == nullptr)
    {
        return core::Result<size_t>::error("no current space");
    }

    auto connection = try$(Asset::by_handle(space, receive->connection_handle));

    if (connection->kind != OBJECT_KIND_IPC_CONNECTION)
    {
        return core::Result<size_t>::error("connection is not an IPC connection");
    }

    auto ipc_connection = connection->ipc_connection;

    if(!ipc_connection->accepted)
    {
        return core::Result<size_t>::error("connection is not accepted");
    }

    auto server = try$(Asset::by_handle(space, receive->server_handle));
    if (server->kind != OBJECT_KIND_IPC_SERVER)
    {
        return core::Result<size_t>::error("server is not an IPC server");
    }

    auto kernel_server = server->ipc_server;

    if(ipc_connection->server_handle != kernel_server->handle)
    {
        return core::Result<size_t>::error("connection is not connected to this server");
    }

    auto res = (server_receive_message(kernel_server, ipc_connection));

    if(res.is_error())
    {
        return core::Result<size_t>(res.error());
    }

    

    auto received_message = res.unwrap();

    if(received_message.is_null)
    {
        receive->returned_msg_handle = 0;
        receive->returned_message = {};
        receive->contain_response = false;
        return core::Result<size_t>::success(0);
    }

    receive->returned_msg_handle = received_message.uid;
    *try$(syscall_check_ptr(receive->returned_message)) = received_message.message_sended.to_server();
    receive->contain_response = true;
    
    return core::Result<size_t>::success((size_t)received_message.uid);
}

core::Result<size_t> ksyscall_client_receive_reply(SyscallIpcClientReceiveReply* receive)
{
    Space *space = nullptr;
    if (receive->space_handle != 0)
    {
        space = try$(Space::space_by_handle(
            Cpu::current()->currentTask()->space(),
            receive->space_handle));
    }
    else
    {
        space = Cpu::current()->currentTask()->space();
    }

    if (space == nullptr)
    {
        return core::Result<size_t>::error("no current space");
    }

    auto connection = try$(Asset::by_handle(space, receive->connection_handle));

    if (connection->kind != OBJECT_KIND_IPC_CONNECTION)
    {
        return core::Result<size_t>::error("connection is not an IPC connection");
    }

    auto ipc_connection = connection->ipc_connection;

    if(!ipc_connection->accepted)
    {
        return core::Result<size_t>::error("connection is not accepted");
    }

    auto res = client_receive_response(ipc_connection, receive->message);

    if(res.is_error())
    {
        return core::Result<size_t>(res.error());
    }

    auto received_message = res.unwrap();

    if(received_message.is_null)
    {
        
        receive->returned_message = {};
        receive->contain_response = false;
        return core::Result<size_t>::success(0);
    }

    //receive->returned_msg_handle = received_message.uid;
    
    * try$(syscall_check_ptr( receive->returned_message) )= received_message.message_responded.to_client();
    
    receive->contain_response = true;

    return core::Result<size_t>::success((size_t)received_message.uid);
}

core::Result<size_t> ksyscall_ipc_call(SyscallIpcCall* call)
{
    Space *space = nullptr;
    if (call->space_handle != 0)
    {
        space = try$(Space::space_by_handle(
            Cpu::current()->currentTask()->space(),
            call->space_handle));
    }
    else
    {
        space = Cpu::current()->currentTask()->space();
    }

    if (space == nullptr)
    {
        return core::Result<size_t>::error("no current space");
    }

    auto connection = try$(Asset::by_handle(space, call->connection_handle));

    if (connection->kind != OBJECT_KIND_IPC_CONNECTION)
    {
        return core::Result<size_t>::error("connection is not an IPC connection");
    }

    auto ipc_connection = connection->ipc_connection;

    if(!ipc_connection->accepted)
    {
        return core::Result<size_t>::error("connection is not accepted");
    }

    auto res = call_server_and_wait(ipc_connection, call->message);

    if(res.is_error())
    {
        return core::Result<size_t>(res.error());
    }

    auto received_message = res.unwrap();

    *try$(syscall_check_ptr(call->returned_message)) = core::move(received_message);
    call->has_reply = true;
    //call->returned_msg_handle = received_message.uid;
    
    return core::Result<size_t>::success((size_t)0);
}

core::Result<size_t> ksyscall_ipc_accept(SyscallIpcAccept* accept)
{
    Space *space = nullptr;
    if (accept->space_handle != 0)
    {
        space = try$(Space::space_by_handle(
            Cpu::current()->currentTask()->space(),
            accept->space_handle));
    }
    else
    {
        space = Cpu::current()->currentTask()->space();
    }

    if (space == nullptr)
    {
        return core::Result<size_t>::error("no current space");
    }

    auto server = try$(Asset::by_handle(space, accept->server_handle));

    if (server->kind != OBJECT_KIND_IPC_SERVER)
    {
        return core::Result<size_t>::error("server is not an IPC server");
    }

    auto kernel_server = server->ipc_server;

    auto res = server_accept_connection( kernel_server);

    if(res.is_error())
    {
        accept->accepted_connection = false;
        return core::Result<size_t>::success(0);
    }
    
    auto connection = res.unwrap();

    if(connection.asset == nullptr)
    {
        accept->accepted_connection = false;
        return core::Result<size_t>::success(0);
    }
    accept->connection_handle = connection.handle;
    accept->accepted_connection = true;
    
    return core::Result<size_t>::success((size_t)connection.handle);
}

core::Result<size_t> ksyscall_ipc_server_reply(SyscallIpcReply* reply)
{
    Space *space = nullptr;
    if (reply->space_handle != 0)
    {
        space = try$(Space::space_by_handle(
            Cpu::current()->currentTask()->space(),
            reply->space_handle));
    }
    else
    {
        space = Cpu::current()->currentTask()->space();
    }

    if (space == nullptr)
    {
        return core::Result<size_t>::error("no current space");
    }

    auto connection = try$(Asset::by_handle(space, reply->connection_handle));

    if (connection->kind != OBJECT_KIND_IPC_CONNECTION)
    {
        return core::Result<size_t>::error("connection is not an IPC connection");
    }

    auto ipc_connection = connection->ipc_connection;

    if(!ipc_connection->accepted)
    {
        return core::Result<size_t>::error("connection is not accepted");
    }

    auto res = server_reply_message(ipc_connection, reply->message_handle, try$(syscall_check_ptr(reply->message)));

    if(res.is_error())
    {
        return core::Result<size_t>(res.error());
    }

    return core::Result<size_t>::success(0);
}


core::Result<size_t> ksyscall_ipc_status(SyscallIpcStatus* status)
{
    Space *space = nullptr;
    if (status->space_handle != 0)
    {
        space = try$(Space::space_by_handle(
            Cpu::current()->currentTask()->space(),
            status->space_handle));
    }
    else
    {
        space = Cpu::current()->currentTask()->space();
    }

    if (space == nullptr)
    {
        return core::Result<size_t>::error("no current space");
    }

    auto connection = try$(Asset::by_handle(space, status->connection_handle));

    if (connection->kind != OBJECT_KIND_IPC_CONNECTION)
    {
        return core::Result<size_t>::error("connection is not an IPC connection");
    }

    auto ipc_connection = connection->ipc_connection;

    if(!ipc_connection->accepted)
    {
        status->returned_is_accepted = false;
    }

    status->returned_is_accepted = true;
    
    return core::Result<size_t>::success(0);
}

core::Result<size_t> syscall_handle(SyscallInterface syscall)
{
    switch (syscall.id)
    {
    case SYSCALL_DEBUG_LOG_ID:
    {
        auto debug = syscall_debug_decode(syscall);
        log::log("{}", debug.message);
        return core::Result<size_t>::success(0);
    }
    case SYSCALL_PHYSICAL_MEM_OWN_ID:
    {
        SyscallMemOwn *mem_own = try$(syscall_check_ptr<SyscallMemOwn>(syscall.arg1));

        return ksyscall_mem_own(mem_own);
    }
    case SYSCALL_MAPPING_CREATE_ID:
    {
        SyscallMap *map = try$(syscall_check_ptr<SyscallMap>(syscall.arg1));

        return ksyscall_map(map);
    }
    case SYSCALL_TASK_CREATE_ID:
    {
        SyscallTaskCreate *task_create = try$(syscall_check_ptr<SyscallTaskCreate>(syscall.arg1));
        return ksyscall_task_create(task_create);
    }
    case SYSCALL_SPACE_CREATE_ID:
    {
        SyscallSpaceCreate *space_create = try$(syscall_check_ptr<SyscallSpaceCreate>(syscall.arg1));
        return ksyscall_space_create(space_create);
    }
    case SYSCALL_ASSET_RELEASE_ID:
    {
        SyscallAssetRelease *asset_release = try$(syscall_check_ptr<SyscallAssetRelease>(syscall.arg1));
        return ksyscall_asset_release(asset_release);
    }
    case SYSCALL_TASK_LAUNCH_ID:
    {
        SyscallTaskLaunch *task_launch = try$(syscall_check_ptr<SyscallTaskLaunch>(syscall.arg1));
        return ksyscall_task_launch(task_launch);
    }
    case SYSCALL_ASSET_MOVE:
    {
        SyscallAssetMove *asset_move = try$(syscall_check_ptr<SyscallAssetMove>(syscall.arg1));
        return ksyscall_asset_move(asset_move);
    }
    case SYSCALL_IPC_CREATE_SERVER_ID:
    {
        SyscallIpcCreateServer *create = try$(syscall_check_ptr<SyscallIpcCreateServer>(syscall.arg1));
        return ksyscall_create_server(create);
    }
    case SYSCALL_IPC_CONNECT_ID:
    {
        SyscallIpcConnect *create = try$(syscall_check_ptr<SyscallIpcConnect>(syscall.arg1));
        return ksyscall_create_connection(create);
    }
    case SYSCALL_IPC_SEND_ID:
    {
        SyscallIpcSend *send = try$(syscall_check_ptr<SyscallIpcSend>(syscall.arg1));
        return ksyscall_send(send);
    }
    case SYSCALL_IPC_SERVER_RECEIVE_ID:
    {
        SyscallIpcServerReceive *receive = try$(syscall_check_ptr<SyscallIpcServerReceive>(syscall.arg1));
        return ksyscall_server_receive(receive);
    }
    case SYSCALL_IPC_CLIENT_RECEIVE_REPLY_ID:
    {
        SyscallIpcClientReceiveReply *receive = try$(syscall_check_ptr<SyscallIpcClientReceiveReply>(syscall.arg1));
        return ksyscall_client_receive_reply(receive);
    }
    case SYSCALL_IPC_CALL_ID:
    {
        SyscallIpcCall *call = try$(syscall_check_ptr<SyscallIpcCall>(syscall.arg1));
        return ksyscall_ipc_call(call);
    }
    case SYSCALL_IPC_ACCEPT_ID:
    {
        SyscallIpcAccept *accept = try$(syscall_check_ptr<SyscallIpcAccept>(syscall.arg1));
        return ksyscall_ipc_accept(accept);
    }
    case SYSCALL_IPC_REPLY_ID:
    {
        SyscallIpcReply *reply = try$(syscall_check_ptr<SyscallIpcReply>(syscall.arg1));
        return ksyscall_ipc_server_reply(reply);
    }
    case SYSCALL_IPC_STATUS_ID:
    {
        SyscallIpcStatus *status = try$(syscall_check_ptr<SyscallIpcStatus>(syscall.arg1));
        return ksyscall_ipc_status(status);
    }
    
    
    default:
        return {"Unknown syscall ID"};
    }
}
