#include "arch/generic/syscalls.h"

uintptr_t syscall_execute(uint32_t id, uintptr_t arg1, uintptr_t arg2, uintptr_t arg3, uintptr_t arg4, uintptr_t arg5, uintptr_t arg6)
{
/*     .id = (uint32_t)stackframe->rax,
    .arg1 = stackframe->rbx,
    .arg2 = stackframe->rdx,
    .arg3 = stackframe->rsi,
    .arg4 = stackframe->rdi,
    .arg5 = stackframe->r8,
    .arg6 = stackframe->r9,*/


    register uintptr_t out0 asm("rax") = id;

    register uintptr_t _arg1 asm("rbx") = arg1;
    register uintptr_t _arg2 asm("rdx") = arg2;
    register uintptr_t _arg3 asm("rsi") = arg3;
    register uintptr_t _arg4 asm("rdi") = arg4;
    register uintptr_t _arg5 asm("r8") = arg5;
    register uintptr_t _arg6 asm("r9") = arg6;

    asm volatile(
        "syscall \n"
        : "=r"(out0)
        : "r"(out0), "r"(_arg1), "r"(_arg2), "r"(_arg3), "r"(_arg4), "r"(_arg5), "r"(_arg6)
        : "memory", "rcx", "r11");

    return out0;
}
