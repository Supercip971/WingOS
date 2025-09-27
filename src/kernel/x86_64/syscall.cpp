#include "syscall.hpp"

#include "arch/x86_64/context.hpp"
#include "arch/x86_64/gdt.hpp"
#include "arch/x86_64/msr.hpp"

#include "kernel/generic/cpu.hpp"
#include "kernel/generic/kernel.hpp"
#include "kernel/generic/syscalls.hpp"
#include "libcore/alloc/alloc.hpp"
#include "libcore/fmt/log.hpp"
#include "libcore/lock/lock.hpp"

namespace arch::amd64
{

core::Lock sys_lock;
extern "C" uint64_t syscall_higher_handler(SyscallStackFrame *stackframe)
{
    //  log("syscall", LOG_INFO, "called syscall higher handler in: {} (stack: {})", stackframe->rip, stackframe->rsp);

    auto res = syscall_handle({
        .id = (uint32_t)stackframe->rax,
        .arg1 = stackframe->rbx,
        .arg2 = stackframe->rdx,
        .arg3 = stackframe->rsi,
        .arg4 = stackframe->rdi,
        .arg5 = stackframe->r8,
        .arg6 = stackframe->r9,
    });

    if (res.is_error())
    {
        log::err$("syscall error: {}", res.error());
        stackframe->rax = -1;
    }
    else
    {
        stackframe->rax = res.unwrap();
    }

    //  log("syscall", LOG_INFO, "exited syscall higher handler with result: {}", ret);
    return res.unwrap();
}

extern "C" void syscall_handle();

//  - Entry CS  = KERNEL_CODE_SELECTOR
//  - Entry SS  = KERNEL_CODE_SELECTOR + 8  = KERNEL_DATA_SELECTOR
//  - Return CS = USER32_CODE_SELECTOR + 16 = USER_CODE_SELECTOR
//  - Return SS = USER32_CODE_SELECTOR + 8  = USER_DATA_SELECTOR
core::Result<void> syscall_init_for_current_cpu()
{

    Msr::Write(MsrReg::EFER, Msr::Read(MsrReg::EFER) | 1); // turn on syscall

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
    Msr::Write(MsrReg::STAR, ((((uint64_t)Gdt::kernel_code_segment_id * 8)) << 32) | (((uint64_t)((Gdt::kernel_data_segment_id * 8) | 3)) << 48));
    Msr::Write(MsrReg::LSTAR, (uint64_t)syscall_handle);
    Msr::Write(MsrReg::SYSCALL_FLAG_MASK, (uint64_t)(1 << 9) | 0xfffffffe);

    Cpu::current()->syscall_stack = (void *)((uintptr_t)try$(core::mem_alloc(kernel::kernel_stack_size)) +
                                             kernel::kernel_stack_size - 16); // allocate a stack for syscall handling
    return {};
}

} // namespace arch::amd64