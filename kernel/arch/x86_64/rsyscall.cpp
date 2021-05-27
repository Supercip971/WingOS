#include "rsyscall.h"
#include "64bit.h"
#include "gdt.h"

#include "syscall.h"

extern "C" uint64_t syscall_higher_handler(cpu_registers *stackframe)
{
    //  log("syscall", LOG_INFO, "called syscall higher handler in: {} (stack: {})", stackframe->rip, stackframe->rsp);
    uint64_t ret = syscall(stackframe->rax, stackframe->rbx, stackframe->rdx, stackframe->rsi, stackframe->rdi, 0, stackframe);
    //  log("syscall", LOG_INFO, "exited syscall higher handler with result: {}", ret);
    return ret;
}

extern "C" void syscall_handle();

//  - Entry CS  = KERNEL_CODE_SELECTOR
//  - Entry SS  = KERNEL_CODE_SELECTOR + 8  = KERNEL_DATA_SELECTOR
//  - Return CS = USER32_CODE_SELECTOR + 16 = USER_CODE_SELECTOR
//  - Return SS = USER32_CODE_SELECTOR + 8  = USER_DATA_SELECTOR
void init_syscall_for_current_cpu()
{

    x86_wrmsr(EFER, x86_rdmsr(EFER) | 1); // turn on syscall

    // when starting the syscall:
    // CS = kcode
    // SS = kcode + 8
    // when returning:
    // cs=  ucode + 16
    // ss = ucode + 8

    // so we need to have:
    // kcode : kernel code
    // kcode + 8: kernel data
    // ucode + 8 : user data
    // ucode + 16 : user code
    x86_wrmsr(STAR, ((uint64_t)gdt_selector::KERNEL_CODE << 32) | ((uint64_t)(gdt_selector::KERNEL_DATA | 3) << 48));
    x86_wrmsr(LSTAR, (uint64_t)syscall_handle);
    x86_wrmsr(SYSCALL_FLAG_MASK, (uint64_t)(1 << 9));
}
