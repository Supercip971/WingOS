#include "arch/generic/syscalls.h"

extern "C" uintptr_t syscall_execute(uint32_t id, uintptr_t arg1, uintptr_t arg2, uintptr_t arg3, uintptr_t arg4, uintptr_t arg5, uintptr_t arg6)
{
    uintptr_t kernel_return;
    asm volatile(
        "push %%r11 \n"
        "push %%rcx \n"
        "syscall \n"
        "pop %%rcx \n"
        "pop %%r11 \n"
        : "=a"(kernel_return)
        : "a"(id), "b"(arg1), "d"(arg2), "S"(arg3), "D"(arg4), "r"(arg5), "r"(arg6)
        : "memory", "rcx", "r11"); // for debugging
    return kernel_return;
}
