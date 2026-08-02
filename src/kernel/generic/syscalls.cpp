#include "syscalls.hpp"

#include "arch/x86_64/paging.hpp"
#include "hw/mem/addr_space.hpp"
#include "kernel/generic/asset_types.hpp"
#include "kernel/generic/ipc_asset.hpp"

#include "arch/x86/port.hpp"
#include "kernel/generic/asset.hpp"
#include "kernel/generic/cpu.hpp"
#include "kernel/generic/ipc.hpp"
#include "kernel/generic/scheduler.hpp"
#include "kernel/generic/space.hpp"
#include "kernel/generic/task.hpp"
#include "libcore/fmt/flags.hpp"
#include "libcore/fmt/log.hpp"
#include "libcore/lock/lock.hpp"
#include "libcore/result.hpp"
#include "libcore/type-utils.hpp"
#include "wingos-headers/asset.h"
#include "wingos-headers/syscalls.h"

fc::Lock log_lock;

template <typename T>
fc::Result<T *> syscall_check_ptr(kernel::Task *caller, uintptr_t ptr)
{
    if (ptr == 0)
    {
        return fc::Result<T *>::error("null pointer");
    }

    if (caller == nullptr)
    {
        return fc::Result<T *>::error("no current task");
    }

    if (ptr >= MMAP_IO_BASE)
    {
        return fc::Result<T *>::error("invalid pointer");
    }

    try$(caller->vmm_space().verify(ptr, sizeof(T)));

    return reinterpret_cast<T *>(ptr);
}

template <typename T>
fc::Result<T *> syscall_check_ptr(kernel::Task *caller, T *ptr)
{
    if (ptr == 0)
    {
        return fc::Result<T *>::error("null pointer");
    }

    auto tsk = caller;
    if (tsk == nullptr)
    {
        return fc::Result<T *>::error("no current task");
    }

    try$(tsk->vmm_space().verify((uintptr_t)ptr, sizeof(T)));

    return reinterpret_cast<T *>(ptr);
}

fc::Result<uintptr_t> ksyscall_mem_own(kernel::Task *caller, SyscallMemOwn *mem_own)
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
        return fc::Result<uintptr_t>::error("no current space");
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

fc::Result<uintptr_t> ksyscall_map(kernel::Task *caller, SyscallMap *map)
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
        return fc::Result<uintptr_t>::error("no current space");
    }

    // Detailed logging to diagnose occasional start>=end mapping requests.
    // This should help pinpoint the caller and whether the SyscallMap struct
    // was corrupted or simply passed invalid values.
    if (map->start >= map->end)
    {
        fmt::err$("ksyscall_map: invalid range start>=end (start={}, end={})",
                  map->start | fmt::FMT_HEX,
                  map->end | fmt::FMT_HEX);
        return fc::Result<uintptr_t>::error("invalid mapping range");
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

fc::Result<size_t> ksyscall_task_create(kernel::Task *caller, SyscallTaskCreate *task_create)
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
        return fc::Result<size_t>::error("no current space");
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

fc::Result<size_t> ksyscall_space_create(kernel::Task *caller, SyscallSpaceCreate *args)
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
        return fc::Result<size_t>::error("no current space");
    }

    auto asset = try$(space->create_space(args->flags, args->rights));

    args->returned_handle = asset.handle;
    return (uint64_t)asset.handle;
}

fc::Result<size_t> ksyscall_mem_release(kernel::Task *caller, SyscallAssetRelease *release)
{
    auto space = caller->space();
    if (space == nullptr)
    {
        return fc::Result<size_t>::error("no current space");
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
        return fc::Result<size_t>::error("memory asset not found for given address range");
    }

    space->asset_release(virt_mem_asset);
    space->asset_release(mem_asset);
    return 0ul;
}

fc::Result<size_t> ksyscall_asset_release(kernel::Task *caller, SyscallAssetRelease *release)
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
        return fc::Result<size_t>::error("no current space");
    }

    auto asset = try$(space->by_handle<Asset>(release->asset_handle));
    space->asset_release(asset);

    return 0ul;
}

fc::Result<size_t> ksyscall_task_launch(kernel::Task *caller, SyscallTaskLaunch *task_launch)
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
        return fc::Result<size_t>::error("no current space");
    }

    auto task_asset = try$(space->by_handle<AssetTask>(task_launch->task_handle));

    if (task_asset.asset == nullptr)
    {
        return fc::Result<size_t>::error("task asset has no task");
    }

    try$(kernel::task_run(task_asset->uid()));

    return 0ul;
}

fc::Result<size_t> ksyscall_asset_move(kernel::Task *caller, SyscallAssetMove *asset_move_args)
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
        return fc::Result<size_t>::error("no current space");
    }

    auto asset = try$(from_space->by_handle<Asset>(asset_move_args->asset_handle));

    auto moved_asset = try$(Space::asset_move(from_space, to_space, asset));

    asset_move_args->returned_handle_in_space = moved_asset.handle;

    return (uint64_t)moved_asset.handle;
}

fc::Result<size_t> ksyscall_create_endpoint(kernel::Task *caller, SyscallIpcCreateEndpoint *create)
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
        return fc::Result<size_t>::error("no current space");
    }

    auto asset = try$(space->create_ipc_endpoint({
        .publish = create->publish,
        .is_root = create->is_root,
    }));

    // Userspace expects the server handle as an "addr" out-param.
    create->returned_addr = (uintptr_t)asset->uuid;
    create->returned_handle = asset.handle;

    return (uint64_t)asset.handle;
}

fc::Result<size_t> ksyscall_create_connection(kernel::Task *caller, SyscallIpcConnect *create)
{

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
        return fc::Result<size_t>::error("no current space");
    }

    AssetRef<kernel::IpcEndpointConnection> conn{};
    if (create->connect_by_address)
    {
        auto endpoint = query_server(create->server_address);
        if (endpoint.is_error())
        {
            fmt::err$("server not found: {}", endpoint.error());
            return fc::Result<size_t>::error(endpoint.error());
        }
        conn = try$(space->create_ipc_connection((AssetIpcConnectionCreateParams){
            .endpoint = endpoint.unwrap(),
        }));
    }
    else
    {
        auto endpoint = try$(space->by_handle<kernel::IpcEndpoint>(create->endpoint_handle));

        conn = try$(space->create_ipc_connection((AssetIpcConnectionCreateParams){
            .endpoint = endpoint,
        }));
    }

    create->returned_handle_sender = conn.handle;
    create->port_used = conn->port;
    return (uint64_t)conn.handle;
}

fc::Result<size_t> ksyscall_send(kernel::Task *caller, SyscallIpcSend *send)
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
    AssetRef<Space> space_ref = AssetRef<Space>(space, -1);
    AssetRef<AssetTask> return_task = AssetRef<AssetTask>(caller, -1);

    if (space == nullptr)
    {
        return fc::Result<size_t>::error("no current space");
    }

    auto connection = try$(space->by_handle<kernel::IpcEndpointConnection>(send->connection_handle));

    if (send->async)
    {
        try$(kernel::ipc_send_async(space_ref, return_task, connection, send->message));
    }
    else
    {
        try$(kernel::ipc_send(space_ref, return_task, connection, send->message, false));
    }

    return (size_t)0;
}

fc::Result<size_t> ksyscall_receive(kernel::Task *caller, SyscallIpcReceive *receive)
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
        return fc::Result<size_t>::error("no current space");
    }

    AssetRef<Space> space_ref = AssetRef<Space>(space, -1);
    AssetRef<AssetTask> return_task = AssetRef<AssetTask>(caller, -1);

    auto endpoint = (space->by_handle<kernel::IpcEndpoint>(receive->endpoint_handle)).take();

    if (receive->async)
    {
        try$(kernel::ipc_receive_async(space_ref, endpoint, receive->returned_message, &receive->return_context_handle));
    }
    else
    {
        try$(kernel::ipc_receive(space_ref, return_task, endpoint, receive->returned_message, &receive->return_context_handle));
    }
    return {};
}

fc::Result<size_t> ksyscall_ipc_call(kernel::Task *caller, SyscallIpcCall *call)
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
    AssetRef<Space> space_ref = AssetRef<Space>(space, -1);
    AssetRef<AssetTask> return_task = AssetRef<AssetTask>(caller, -1);

    if (space == nullptr)
    {
        return fc::Result<size_t>::error("no current space");
    }

    auto connection = try$(space->by_handle<kernel::IpcEndpointConnection>(call->connection_handle));

    try$(kernel::ipc_send(space_ref, return_task, connection, call->message, true));

    return 0ul;
}

fc::Result<size_t> ksyscall_ipc_reply(kernel::Task *caller, SyscallIpcReply *reply)
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
        return fc::Result<size_t>::error("no current space");
    }
    AssetRef<Space> space_ref = AssetRef<Space>(space, -1);

    auto ret_task = try$(space->by_handle<kernel::IpcMessageReturnTask>(reply->return_task_handle));

    try$(kernel::ipc_reply(space_ref, ret_task, reply->message));

    return 0ul;
}

fc::Result<size_t> ksyscall_ipc_asset_info(kernel::Task *caller, SyscallAssetInfo *info)
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
        return fc::Result<size_t>::error("no current space");
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
        fmt::warn$("Asset info for kind {} not implemented", asset.asset->kind);
        break;
    }

    return 0ul;
}

fc::Result<size_t> ksyscall_ipc_x86_port(kernel::Task *caller, SyscallIpcX86Port *port)
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
        return fc::Result<size_t>::error("no current space");
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
            fmt::err$("Invalid port size: {}", port->size);
            fmt::err$("Port: {}", port->port);
            return fc::Result<size_t>::error("invalid size");
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
            fmt::err$("Invalid port size: {}", port->size);
            fmt::err$("Port: {}", port->port);
            fmt::err$("Data: {}", port->data);
            return fc::Result<size_t>::error("invalid size");
        }
    }

    return 0ul;
}

fc::Result<size_t> syscall_handle(SyscallInterface syscall, kernel::Task *caller)
{
    switch (syscall.id)
    {
    case SYSCALL_DEBUG_LOG_ID:
    {
        fmt::log_lock();
        auto debug = syscall_debug_decode(syscall);
        fmt::log("{}", caller->uid());
        fmt::log("{}", debug.message);
        fmt::log_release();

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
    case SYSCALL_IPC_CREATE_ENDPOINT_ID:
    {
        SyscallIpcCreateEndpoint *create = try$(syscall_check_ptr<SyscallIpcCreateEndpoint>(caller, syscall.arg1));
        return ksyscall_create_endpoint(caller, create);
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
    case SYSCALL_IPC_RECEIVE_ID:
    {
        SyscallIpcReceive *receive = try$(syscall_check_ptr<SyscallIpcReceive>(caller, syscall.arg1));
        return ksyscall_receive(caller, receive);
    }
    case SYSCALL_IPC_CALL_ID:
    {
        SyscallIpcCall *call = try$(syscall_check_ptr<SyscallIpcCall>(caller, syscall.arg1));
        return ksyscall_ipc_call(caller, call);
    }
    case SYSCALL_IPC_REPLY_ID:
    {
        SyscallIpcReply *reply = try$(syscall_check_ptr<SyscallIpcReply>(caller, syscall.arg1));
        return ksyscall_ipc_reply(caller, reply);
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
