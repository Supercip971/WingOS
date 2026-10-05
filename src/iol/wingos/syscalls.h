#pragma once

#include <arch/generic/syscalls.h>

#include "wingos-headers/asset.h"
#include "wingos-headers/ipc.h"
#include "wingos-headers/syscalls.h"

#ifdef __cplusplus
extern "C"
{
#endif
    static inline uintptr_t sys$debug_log(const char *message)
    {
        SyscallInterface interface = syscall_debug_encode({(char *)message});
        return syscall_execute(interface.id, interface.arg1, interface.arg2, interface.arg3, interface.arg4, interface.arg5, interface.arg6);
    }

    static inline SyscallMemOwn sys$mem_own(AssetHandle target_space_handle, size_t size, size_t addr)
    {
        SyscallMemOwn create = {
            target_space_handle, size, addr, AssetHandle::invalid()};
        SyscallInterface interface = syscall_physical_mem_own_encode(&create);
        uintptr_t result = syscall_execute(interface.id, interface.arg1, interface.arg2, interface.arg3, interface.arg4, interface.arg5, interface.arg6);
        (void)result;

        return create;
    }

    static inline SyscallMap sys$map_create(AssetHandle target_space_handle, size_t start, size_t end, AssetHandle physical_mem_handle, uint64_t flags)
    {
        SyscallMap create = {target_space_handle, start, end, physical_mem_handle, flags, AssetHandle::invalid()};
        SyscallInterface interface = syscall_map_encode(&create);
        uintptr_t result = syscall_execute(interface.id, interface.arg1, interface.arg2, interface.arg3, interface.arg4, interface.arg5, interface.arg6);
        (void)result;

        return create;
    }

    static inline SyscallTaskCreate sys$task_create(AssetHandle target_space_handle, uint64_t launch, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4)
    {
        SyscallTaskCreate create = {target_space_handle, launch, {0, 0, 0, 0}, AssetHandle::invalid()};

        create.args[0] = arg1;
        create.args[1] = arg2;
        create.args[2] = arg3;
        create.args[3] = arg4;

        SyscallInterface interface = syscall_task_create_encode(&create);
        uintptr_t result = syscall_execute(interface.id, interface.arg1, interface.arg2, interface.arg3, interface.arg4, interface.arg5, interface.arg6);
        (void)result;

        return create;
    }

    static inline SyscallSpaceCreate sys$space_create(AssetHandle parent_space_handle, uint64_t flags, uint64_t rights)
    {
        SyscallSpaceCreate create = {parent_space_handle, flags, rights, AssetHandle::invalid()};
        SyscallInterface interface = syscall_space_create_encode(&create);
        uintptr_t result = syscall_execute(interface.id, interface.arg1, interface.arg2, interface.arg3, interface.arg4, interface.arg5, interface.arg6);
        (void)result;

        return create;
    }

    static inline SyscallAssetRelease sys$asset_release(AssetHandle space_handle, AssetHandle asset_handle)
    {
        SyscallAssetRelease release = {space_handle, asset_handle, NULL, NULL};
        SyscallInterface interface = syscall_asset_release_encode(&release);
        uintptr_t result = syscall_execute(interface.id, interface.arg1, interface.arg2, interface.arg3, interface.arg4, interface.arg5, interface.arg6);
        (void)result;

        return release;
    }

    static inline SyscallAssetRelease sys$asset_release_mem(void *addr, void *end)
    {
        SyscallAssetRelease release = {.space_handle = 0, .asset_handle = AssetHandle::invalid(), .addr = addr, .end = end};
        SyscallInterface interface = syscall_asset_release_encode(&release);
        uintptr_t result = syscall_execute(interface.id, interface.arg1, interface.arg2, interface.arg3, interface.arg4, interface.arg5, interface.arg6);
        (void)result;

        return release;
    }

    static inline SyscallTaskLaunch sys$task_launch(AssetHandle target_space_handle, AssetHandle task_handle, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4)
    {
        SyscallTaskLaunch launch = {target_space_handle, task_handle, {arg1, arg2, arg3, arg4}};
        SyscallInterface interface = syscall_task_launch_encode(&launch);
        uintptr_t result = syscall_execute(interface.id, interface.arg1, interface.arg2, interface.arg3, interface.arg4, interface.arg5, interface.arg6);
        (void)result;
        return launch;
    }

    static inline SyscallAssetMove sys$asset_move(AssetHandle from_space_handle, AssetHandle to_space_handle, AssetHandle asset_handle)
    {
        SyscallAssetMove move = {from_space_handle, to_space_handle, asset_handle, 0, AssetHandle::invalid()};
        SyscallInterface interface = syscall_asset_move_encode(&move);
        uintptr_t result = syscall_execute(interface.id, interface.arg1, interface.arg2, interface.arg3, interface.arg4, interface.arg5, interface.arg6);
        (void)result;
        return move;
    }

    static inline SyscallAssetMove sys$asset_copy(AssetHandle from_space_handle, AssetHandle to_space_handle, AssetHandle asset_handle)
    {
        SyscallAssetMove copy = {from_space_handle, to_space_handle, asset_handle, 1, AssetHandle::invalid()};
        SyscallInterface interface = syscall_asset_move_encode(&copy);
        uintptr_t result = syscall_execute(interface.id, interface.arg1, interface.arg2, interface.arg3, interface.arg4, interface.arg5, interface.arg6);
        (void)result;
        return copy;
    }

    static inline SyscallIpcCreateEndpoint sys$ipc_create_endpoint(AssetHandle space_handle, bool is_root, bool publish)
    {
        SyscallIpcCreateEndpoint create = {space_handle, is_root, publish, 0, AssetHandle::invalid()};
        SyscallInterface interface = syscall_ipc_create_endpoint_encode(&create);
        uintptr_t result = syscall_execute(interface.id, interface.arg1, interface.arg2, interface.arg3, interface.arg4, interface.arg5, interface.arg6);
        (void)result;
        return create;
    }

    static inline SyscallIpcConnect sys$ipc_connect(AssetHandle space_handle, bool is_using_server_address, uint64_t id)
    {
        SyscallIpcConnect connect = {space_handle, is_using_server_address, 0, AssetHandle::invalid(), 0};

        if (is_using_server_address)
        {
            connect.server_address = id;
        }
        else
        {
            connect.endpoint_handle = id;
        }

        SyscallInterface interface = syscall_ipc_connect_encode(&connect);
        uintptr_t result = syscall_execute(interface.id, interface.arg1, interface.arg2, interface.arg3, interface.arg4, interface.arg5, interface.arg6);
        (void)result;
        return connect;
    }

    static inline SyscallIpcConnect sys$ipc_connect_raw(SyscallIpcConnect connect)
    {
        SyscallInterface interface = syscall_ipc_connect_encode(&connect);
        uintptr_t result = syscall_execute(interface.id, interface.arg1, interface.arg2, interface.arg3, interface.arg4, interface.arg5, interface.arg6);
        (void)result;
        return connect;
    }

    static inline SyscallIpcSend sys$ipc_send(AssetHandle space_handle, IpcConnectionHandle connection_handle, IpcMessage *message, bool async)
    {
        SyscallIpcSend send = {
            .space_handle = space_handle,
            .async = async,
            .connection_handle = connection_handle,
            .message = message,
        };
        SyscallInterface interface = syscall_ipc_send_encode(&send);
        uintptr_t result = syscall_execute(interface.id, interface.arg1, interface.arg2, interface.arg3, interface.arg4, interface.arg5, interface.arg6);
        (void)result;
        return send;
    }

    static inline SyscallIpcReceive sys$ipc_wait_for_interrupt(uint64_t internal_signal_id)
    {
        SyscallIpcReceive receive = {
            SYSCALL_IPC_RECEIVE_INTERNAL_SPACE,
            {internal_signal_id},
            0,
            0,
            {AssetHandle::invalid()},
        };
        SyscallInterface interface = syscall_ipc_receive_encode(&receive);
        uintptr_t result = syscall_execute(interface.id, interface.arg1, interface.arg2, interface.arg3, interface.arg4, interface.arg5, interface.arg6);
        (void)result;
        return receive;
    }

    static inline SyscallIpcReceive sys$ipc_wait_for_interrupt_async(uint64_t internal_signal_id)
    {
        SyscallIpcReceive receive = {
            SYSCALL_IPC_RECEIVE_INTERNAL_SPACE,
            {internal_signal_id},
            1,
            0,
            {AssetHandle::invalid()},
        };
        SyscallInterface interface = syscall_ipc_receive_encode(&receive);
        uintptr_t result = syscall_execute(interface.id, interface.arg1, interface.arg2, interface.arg3, interface.arg4, interface.arg5, interface.arg6);
        (void)result;
        return receive;
    }

    static inline SyscallIpcReceive sys$ipc_receive(AssetHandle space_handle, AssetHandle endpoint_handle, IpcMessage *returned_message, bool async)
    {
        SyscallIpcReceive receive = {
            space_handle,
            {endpoint_handle},
            async,
            returned_message,
            {AssetHandle::invalid()},
        };
        SyscallInterface interface = syscall_ipc_receive_encode(&receive);
        uintptr_t result = syscall_execute(interface.id, interface.arg1, interface.arg2, interface.arg3, interface.arg4, interface.arg5, interface.arg6);
        (void)result;
        return receive;
    }

    static inline SyscallIpcCall sys$ipc_call(AssetHandle space_handle, IpcConnectionHandle connection_handle, IpcMessage *message)
    {
        SyscallIpcCall send = {
            .space_handle = space_handle,
            .connection_handle = connection_handle,
            .message = message,
        };
        SyscallInterface interface = syscall_ipc_call_encode(&send);
        uintptr_t result = syscall_execute(interface.id, interface.arg1, interface.arg2, interface.arg3, interface.arg4, interface.arg5, interface.arg6);
        (void)result;
        return send;
    }

    static inline SyscallIpcReply sys$ipc_reply(AssetHandle space_handle, AssetHandle reply_object_handle, IpcMessage *message)
    {
        SyscallIpcReply reply = {
            .space_handle = space_handle,
            .return_task_handle = reply_object_handle,
            .message = message,
        };
        // SyscallIpcReply reply = {space_handle, server_handle, connection_handle, message_handle, message};
        SyscallInterface interface = syscall_ipc_reply_encode(&reply);
        uintptr_t result = syscall_execute(interface.id, interface.arg1, interface.arg2, interface.arg3, interface.arg4, interface.arg5, interface.arg6);
        (void)result;
        return reply;
    }

    static inline SyscallAssetInfo sys$ipc_asset_info_by_handle(AssetHandle space_handle, AssetHandle asset_handle)
    {
        SyscallAssetInfo info = {space_handle, false, {asset_handle}, AssetKind::OBJECT_KIND_UNKNOWN, AssetHandle::invalid(), {}};
        SyscallInterface interface = syscall_ipc_asset_info_encode(&info);
        uintptr_t result = syscall_execute(interface.id, interface.arg1, interface.arg2, interface.arg3, interface.arg4, interface.arg5, interface.arg6);
        (void)result;
        return info;
    }

    static inline SyscallAssetInfo sys$ipc_asset_info_by_index(AssetHandle space_handle, uint64_t asset_index)
    {
        SyscallAssetInfo info = {space_handle, true, {asset_index}, AssetKind::OBJECT_KIND_UNKNOWN, AssetHandle::invalid(), {}};
        SyscallInterface interface = syscall_ipc_asset_info_encode(&info);
        uintptr_t result = syscall_execute(interface.id, interface.arg1, interface.arg2, interface.arg3, interface.arg4, interface.arg5, interface.arg6);
        (void)result;
        return info;
    }

    static inline void sys$ipc_x86_port_out(uint16_t port, uint8_t size, uint32_t data)
    {
        SyscallIpcX86Port port_data = {
            .space_handle = 0, // current space
            .size = size,
            .port = port,
            .write = 1,
            .read = 0,
            .data = data,
            .returned_value = 0};
        SyscallInterface interface = syscall_ipc_x86_port(&port_data);
        uintptr_t result = syscall_execute(interface.id, interface.arg1, interface.arg2, interface.arg3, interface.arg4, interface.arg5, interface.arg6);
        (void)result;
    }

    static inline uint64_t sys$ipc_x86_port_in(uint16_t port, uint8_t size)
    {
        SyscallIpcX86Port port_data = {
            .space_handle = 0, // current space
            .size = size,
            .port = port,
            .write = 0,
            .read = 1,
            .data = 0,
            .returned_value = 0};
        SyscallInterface interface = syscall_ipc_x86_port(&port_data);
        uintptr_t result = syscall_execute(interface.id, interface.arg1, interface.arg2, interface.arg3, interface.arg4, interface.arg5, interface.arg6);
        (void)result;
        return port_data.returned_value;
    }

#ifdef __cplusplus
}
#endif
