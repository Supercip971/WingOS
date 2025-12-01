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
#include "wingos-headers/asset.h"
#include "kernel/generic/space.hpp"

namespace arch::amd64
{
struct stackframe
{
    stackframe *rbp;
    uint64_t rip;
} __attribute__((packed));


void dump_stackframe(void *rbp)
{

    stackframe *frame = reinterpret_cast<stackframe *>(rbp);
    int size = 0;
    while (frame && size++ < 20)
    {
        log::log$("stackframe: {}", frame->rip | fmt::FMT_HEX);
        frame = frame->rbp;
    }

    if(size >= 20)
    {
        log::log$("... (stackframe too deep)");
    }
}


extern "C" uint64_t syscall_higher_handler(SyscallStackFrame *sf)
{
    //  log("syscall", LOG_INFO, "called syscall higher handler in: {} (stack: {})", stackframe->rip, stackframe->rsp);

    SyscallStackFrame* stackframe = sf;
    
    Cpu::enter_syscall_safe_mode();
    Cpu::current()->debug_saved_syscall_stackframe = stackframe->rbp;

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

        Cpu::end_syscall(); // early end syscall mode, 
        log::err$("syscall error: {}", res.error());
        log::log$("syscall id: {}", stackframe->rax);
        log::log$("syscall args: {}, {}, {}, {}, {}, {}", stackframe->rbx, stackframe->rdx, stackframe->rsi, stackframe->rdi, stackframe->r8, stackframe->r9);
        log::log$("task: {}", Cpu::current()->currentTask() ? Cpu::current()->currentTask()->uid() : -1);
        

        log::log$("task stacktrace dump:");
        dump_stackframe((void *)stackframe->rbp);
            
            
        log::log$("task space({}) dump:", Cpu::current()->currentTask()->space()->uid);

    
        auto space = Cpu::current()->currentTask()->space();
        for(size_t i = 0; i < space->assets.len(); i++)
        {
            log::log$("  Asset[{}]: handle={}, kind={}", i, space->assets[i].handle, assetKind2Str(space->assets[i].asset->kind));
        }
        while(true)
        {}
    }
    else
    {
        sf->rax = res.unwrap();
    }
    asm volatile("cli");


    //log::log$("syscall: {} ", stackframe->rax);
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
    Msr::Write(MsrReg::SYSCALL_FLAG_MASK, ((uint32_t)(RFLAGS_INTERRUPT_ENABLE)) |  0xfffffffe);

    Cpu::current()->syscall_stack = (void *)((uintptr_t)try$(core::mem_alloc(kernel::kernel_stack_size)) +
                                             kernel::kernel_stack_size - 16); // allocate a stack for syscall handling
    
                                             
                                             return {};
}

} // namespace arch::amd64