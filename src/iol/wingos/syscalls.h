#pragma once 

#include <arch/generic/syscalls.h>


static inline uintptr_t sys$debug_log(const char *message)
{
    SyscallInterface interface = syscall_debug_encode({(char *)message});
    return syscall_execute(interface.id, interface.arg1, interface.arg2, interface.arg3, interface.arg4, interface.arg5, interface.arg6);
}