#pragma once 

#include <arch/generic/syscalls.h>

#include "kernel/generic/ipc.hpp"
#include "wingos-headers/ipc.h"
#include "wingos-headers/syscalls.h"

#ifdef __cplusplus
extern "C" {
#endif 
static inline uintptr_t sys$debug_log(const char *message)
{
    SyscallInterface interface = syscall_debug_encode({(char *)message});
    return syscall_execute(interface.id, interface.arg1, interface.arg2, interface.arg3, interface.arg4, interface.arg5, interface.arg6);
}


static inline SyscallMemOwn sys$mem_own(uint64_t target_space_handle, size_t size, size_t addr)
{
    SyscallMemOwn create = {
        target_space_handle, size, addr,  0
    };
    SyscallInterface interface = syscall_physical_mem_own_encode(&create);
    uintptr_t result = syscall_execute(interface.id, interface.arg1, interface.arg2, interface.arg3, interface.arg4, interface.arg5, interface.arg6);
    (void)result;

    return create;
}

static inline SyscallMap sys$map_create(uint64_t target_space_handle, size_t start, size_t end, uint64_t physical_mem_handle, uint64_t flags)
{
    SyscallMap create = {target_space_handle, start, end, physical_mem_handle, flags, 0};
    SyscallInterface interface = syscall_map_encode(&create);
    uintptr_t result = syscall_execute(interface.id, interface.arg1, interface.arg2, interface.arg3, interface.arg4, interface.arg5, interface.arg6);
        (void)result;


    return create;
}

static inline SyscallTaskCreate sys$task_create(uint64_t target_space_handle, uint64_t launch, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4)
{
    SyscallTaskCreate create = {target_space_handle, launch, {0, 0, 0, 0}, 0};
    
    create.args[0] = arg1;
    create.args[1] = arg2;
    create.args[2] = arg3;
    create.args[3] = arg4;

    SyscallInterface interface = syscall_task_create_encode(&create);
    uintptr_t result = syscall_execute(interface.id, interface.arg1, interface.arg2, interface.arg3, interface.arg4, interface.arg5, interface.arg6);
        (void)result;


    return create;
}


static inline SyscallSpaceCreate sys$space_create(uint64_t parent_space_handle, uint64_t flags, uint64_t rights)
{
    SyscallSpaceCreate create = {parent_space_handle, flags, rights, 0};
    SyscallInterface interface = syscall_space_create_encode(&create);
    uintptr_t result = syscall_execute(interface.id, interface.arg1, interface.arg2, interface.arg3, interface.arg4, interface.arg5, interface.arg6);
        (void)result;


    return create;
}

static inline SyscallAssetRelease sys$asset_release(uint64_t space_handle, uint64_t asset_handle)
{
    SyscallAssetRelease release = {space_handle, asset_handle, NULL};
    SyscallInterface interface = syscall_asset_release_encode(&release);
    uintptr_t result = syscall_execute(interface.id, interface.arg1, interface.arg2, interface.arg3, interface.arg4, interface.arg5, interface.arg6);
        (void)result;


    return release;
}

static inline SyscallAssetRelease sys$asset_release_mem(void* addr)
{
    SyscallAssetRelease release = {.space_handle = 0,.asset_handle = 0,.addr = addr};
    SyscallInterface interface = syscall_asset_release_encode(&release);
    uintptr_t result = syscall_execute(interface.id, interface.arg1, interface.arg2, interface.arg3, interface.arg4, interface.arg5, interface.arg6);
        (void)result;


    return release;
}

static inline SyscallTaskLaunch sys$task_launch(uint64_t target_space_handle, uint64_t task_handle, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4)
{
    SyscallTaskLaunch launch = {target_space_handle, task_handle, {arg1, arg2, arg3, arg4}};
    SyscallInterface interface = syscall_task_launch_encode(&launch);
    uintptr_t result = syscall_execute(interface.id, interface.arg1, interface.arg2, interface.arg3, interface.arg4, interface.arg5, interface.arg6);
        (void)result;
    return launch;
}

static inline SyscallAssetMove sys$asset_move(uint64_t from_space_handle, uint64_t to_space_handle, uint64_t asset_handle)
{
    SyscallAssetMove move = {from_space_handle, to_space_handle, asset_handle, 0,0};
    SyscallInterface interface = syscall_asset_move_encode(&move);
    uintptr_t result = syscall_execute(interface.id, interface.arg1, interface.arg2, interface.arg3, interface.arg4, interface.arg5, interface.arg6);
        (void)result;
    return move;
}

static inline SyscallAssetMove sys$asset_copy(uint64_t from_space_handle, uint64_t to_space_handle, uint64_t asset_handle)
{
    SyscallAssetMove copy = {from_space_handle, to_space_handle, asset_handle, 1,0};
    SyscallInterface interface = syscall_asset_move_encode(&copy);
    uintptr_t result = syscall_execute(interface.id, interface.arg1, interface.arg2, interface.arg3, interface.arg4, interface.arg5, interface.arg6);
        (void)result;
    return copy;
}


static inline SyscallIpcCreateServer sys$ipc_create_server(uint64_t space_handle, bool is_root)
{
    SyscallIpcCreateServer create = {space_handle, is_root, 0, 0};
    SyscallInterface interface = syscall_ipc_create_server_encode(&create);
    uintptr_t result = syscall_execute(interface.id, interface.arg1, interface.arg2, interface.arg3, interface.arg4, interface.arg5, interface.arg6);
        (void)result;
    return create;
}

static inline SyscallIpcAccept sys$ipc_accept(bool block, uint64_t space_handle, IpcServerHandle server_handle)
{
    SyscallIpcAccept accept = {block, space_handle, false, server_handle, 0};
    SyscallInterface interface = syscall_ipc_accept_encode(&accept);
    uintptr_t result = syscall_execute(interface.id, interface.arg1, interface.arg2, interface.arg3, interface.arg4, interface.arg5, interface.arg6);
        (void)result;
    return accept;
}

static inline SyscallIpcConnect sys$ipc_connect(bool block, uint64_t space_handle, IpcServerHandle server_handle, uint64_t flags)
{
    SyscallIpcConnect connect = {block, space_handle, server_handle, flags, 0};
    SyscallInterface interface = syscall_ipc_connect_encode(&connect);
    uintptr_t result = syscall_execute(interface.id, interface.arg1, interface.arg2, interface.arg3, interface.arg4, interface.arg5, interface.arg6);
        (void)result;
    return connect;
}

static inline SyscallIpcSend sys$ipc_send(uint64_t space_handle, IpcConnectionHandle connection_handle, IpcMessage* message, bool expect_reply)
{
    SyscallIpcSend send = {space_handle, expect_reply, connection_handle, message, 0};
    SyscallInterface interface = syscall_ipc_send_encode(&send);
    uintptr_t result = syscall_execute(interface.id, interface.arg1, interface.arg2, interface.arg3, interface.arg4, interface.arg5, interface.arg6);
        (void)result;
    return send;
}

static inline SyscallIpcClientReceiveReply sys$ipc_receive_reply_client(bool block, uint64_t space_handle, IpcConnectionHandle connection_handle, MessageHandle message_handle, IpcMessage* returned_message)
{
    SyscallIpcClientReceiveReply receive = {block, false, space_handle, 0, message_handle, connection_handle, returned_message};
    SyscallInterface interface = syscall_ipc_receive_client_reply_encode(&receive);
    uintptr_t result = syscall_execute(interface.id, interface.arg1, interface.arg2, interface.arg3, interface.arg4, interface.arg5, interface.arg6);
        (void)result;
    return receive;
}

static inline SyscallIpcServerReceive sys$ipc_receive_server (bool block, uint64_t space_handle,uint64_t server_asset_handle, IpcConnectionHandle connection_handle, IpcMessage* returned_message)
{
    SyscallIpcServerReceive receive = {block, false, space_handle, server_asset_handle, false, connection_handle, 0, returned_message};
    SyscallInterface interface = syscall_ipc_server_receive_encode(&receive);
    uintptr_t result = syscall_execute(interface.id, interface.arg1, interface.arg2, interface.arg3, interface.arg4, interface.arg5, interface.arg6);
        (void)result;
    return receive;
}

static inline SyscallIpcCall sys$ipc_call(uint64_t space_handle, IpcConnectionHandle connection_handle, IpcMessage* message, IpcMessage* returned)
{
    SyscallIpcCall call = {space_handle, false, connection_handle, message, returned};
    SyscallInterface interface = syscall_ipc_call_encode(&call);
    uintptr_t result = syscall_execute(interface.id, interface.arg1, interface.arg2, interface.arg3, interface.arg4, interface.arg5, interface.arg6);
        (void)result;
    return call;
}

static inline SyscallIpcReply sys$ipc_reply(uint64_t space_handle, uint64_t server_handle, uint64_t connection_handle, MessageHandle message_handle, IpcMessage* message)
{
    SyscallIpcReply reply = {space_handle, server_handle, connection_handle, message_handle, message};
    SyscallInterface interface = syscall_ipc_reply_encode(&reply);
    uintptr_t result = syscall_execute(interface.id, interface.arg1, interface.arg2, interface.arg3, interface.arg4, interface.arg5, interface.arg6);
        (void)result;
    return reply;
}

static inline SyscallIpcStatus sys$ipc_status(uint64_t space_handle, IpcConnectionHandle connection_handle)
{
    SyscallIpcStatus status = {space_handle, connection_handle, 0};
    SyscallInterface interface = syscall_ipc_status_encode(&status);
    uintptr_t result = syscall_execute(interface.id, interface.arg1, interface.arg2, interface.arg3, interface.arg4, interface.arg5, interface.arg6);
        (void)result;
    return status;
}

static inline void sys$ipc_x86_port_out(uint16_t port, uint8_t size, uint32_t data)
{
    SyscallIpcX86Port port_data = {
        .space_handle = 0, // current space
        .port = port,
        .size = size,
        .write = true,
        .data = data,
        .read = false,
        .returned_value = 0
    };
    SyscallInterface interface = syscall_ipc_x86_port(&port_data);
    uintptr_t result = syscall_execute(interface.id, interface.arg1, interface.arg2, interface.arg3, interface.arg4, interface.arg5, interface.arg6);
    (void)result;
}

static inline uint64_t sys$ipc_x86_port_in(uint16_t port, uint8_t size)
{
    SyscallIpcX86Port port_data = {
        .space_handle = 0, // current space
        .port = port,
        .size = size,
        .write = false,
        .data = 0,
        .read = true,
        .returned_value = 0
    };
    SyscallInterface interface = syscall_ipc_x86_port(&port_data);
    uintptr_t result = syscall_execute(interface.id, interface.arg1, interface.arg2, interface.arg3, interface.arg4, interface.arg5, interface.arg6);
    (void)result;
    return port_data.returned_value;
}




#ifdef __cplusplus
}
#endif