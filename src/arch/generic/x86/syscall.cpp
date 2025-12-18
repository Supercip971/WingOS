#include "arch/generic/syscalls.h"


 uintptr_t syscall_execute(uint32_t id, uintptr_t arg1, uintptr_t arg2, uintptr_t arg3, uintptr_t arg4, uintptr_t arg5, uintptr_t arg6)
{
    uintptr_t kernel_return;
    register uintptr_t r8_arg asm("r8") = arg5;
    register uintptr_t r9_arg asm("r9") = arg6;
    asm volatile(
        "syscall \n"
        : "=a"(kernel_return)
        : "a"(id), "b"(arg1), "d"(arg2), "S"(arg3), "D"(arg4), "r"(r8_arg), "r"(r9_arg)
        : "memory", "rcx", "r11"); // for debugging
    return kernel_return;
}