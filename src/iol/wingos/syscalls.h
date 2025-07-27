#pragma once 

#include <arch/generic/syscalls.h>

#include <wingos-headers/syscalls.h>

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
    SyscallMap create = {target_space_handle, start, end, physical_mem_handle, flags};
    SyscallInterface interface = syscall_map_encode(&create);
    uintptr_t result = syscall_execute(interface.id, interface.arg1, interface.arg2, interface.arg3, interface.arg4, interface.arg5, interface.arg6);
        (void)result;


    return create;
}

static inline SyscallTaskCreate sys$task_create(uint64_t target_space_handle, uint64_t launch, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4)
{
    SyscallTaskCreate create = {target_space_handle, launch, {0, 0, 0, 0}};
    
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
    SyscallSpaceCreate create = {parent_space_handle, flags, rights};
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
#ifdef __cplusplus
}
#endif