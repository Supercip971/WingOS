#include <kernel/generic/task.hpp>
#include <libcore/fmt/log.hpp>

#include "arch/x86_64/context.hpp"
#include "arch/x86_64/gdt.hpp"
#include "kernel/x86_64/context.hpp"
#include "kernel/x86_64/cpu.hpp"

#include "kernel/generic/context.hpp"
#include "kernel/generic/kernel.hpp"
#include "kernel/generic/paging.hpp"
#include "libcore/alloc/alloc.hpp"
#include "libcore/lock/lock.hpp"

namespace kernel
{
core::Result<CpuContext *> CpuContext::create_empty()
{

    arch::amd64::CpuContextAmd64 *data = new arch::amd64::CpuContextAmd64{};

    data->lock.release();
    data->await_load = false;

    data->await_save = false;
    data->stackframe(arch::amd64::StackFrame{});
 
    return (CpuContext*)data;
}

void CpuContext::load_to(void *state)
{
    arch::amd64::CpuContextAmd64 const *data = this->as<arch::amd64::CpuContextAmd64>();

    arch::amd64::StackFrame *frame = (arch::amd64::StackFrame *)state;

    // Validate frame before loading
    if (!frame || !data)
    {
        log::err$("Invalid frame or data in load_to");
        return;
    }

    auto stored_frame = data->stackframe();

    // Validate segment selectors
    if (stored_frame.cs == 0 || stored_frame.ss == 0)
    {
     //   log::log$("schedule in ring 0: CS={}, SS={}", stored_frame.cs, stored_frame.ss);
    }

    // Validate stack pointer
    if (stored_frame.rsp == 0 || stored_frame.rip == 0)
    {
        log::err$("Invalid stack pointer: RSP={}", stored_frame.rsp);
        log::err$("Invalid instruction pointer: RIP={}", stored_frame.rip);
        return;
    }

    *frame = stored_frame;

    Cpu::current()->syscall_stack = data->syscall_stack_top;
    Cpu::current()->saved_stack = data->saved_syscall_stack;
    data->simd_context.load();

    this->_vmm_space->use();

    {

        lock_scope$(this->lock);

        this->await_load = false;
    }
}

void CpuContext::dump()
{
    arch::amd64::CpuContextAmd64 const *data = this->as<arch::amd64::CpuContextAmd64>();

    log::log$("Dumping CPU context:");
    log::log$("  Stack Pointer: {}", (uintptr_t)data->stack_ptr | fmt::FMT_HEX);
    log::log$("  Kernel Stack Pointer: {}", (uintptr_t)data->kernel_stack_ptr | fmt::FMT_HEX);
    log::log$("  Stack Frame: {}", data->stackframe());
    log::log$("  Await Save: {}", data->await_save);
    log::log$("  Await Load: {}", data->await_load);
}

void CpuContext::save_in(void *state)
{


    arch::amd64::CpuContextAmd64 *data = this->as<arch::amd64::CpuContextAmd64>();

    arch::amd64::StackFrame *frame = (arch::amd64::StackFrame *)state;

    data->stackframe(*frame);

    this->syscall_stack_top = Cpu::current()->syscall_stack;
    this->saved_syscall_stack = Cpu::current()->saved_stack;
    data->simd_context.save();
    {

        lock_scope$(this->lock);
        this->await_save = false;

    }
}

void CpuContext::release()
{
    arch::amd64::CpuContextAmd64 *data = this->as<arch::amd64::CpuContextAmd64>();

    if (data->stack_ptr != nullptr)
        core::mem_free(data->stack_ptr);
    if (data->kernel_stack_ptr != nullptr)
        core::mem_free(data->kernel_stack_ptr);

    data->simd_context.release();

    data->kernel_stack_ptr = nullptr;
    data->stack_ptr = nullptr;
}

void CpuContext::use_stack_addr(uintptr_t addr)
{
    auto data = this->as<arch::amd64::CpuContextAmd64>();
    data->_user_stack_addr(addr);
}

core::Result<void> CpuContext::prepare(CpuContextLaunch launch)
{

    auto data = this->as<arch::amd64::CpuContextAmd64>();

    data->stack_ptr = try$(core::mem_alloc(kernel::userspace_stack_size));

    data->kernel_stack_ptr = try$(core::mem_alloc(kernel::kernel_stack_size));

    data->syscall_stack_ptr = try$(core::mem_alloc(kernel::kernel_stack_size)); // allocate a stack for syscall handling

    data->stack_top = (void *)((uintptr_t)data->stack_ptr + kernel::userspace_stack_size);
    data->kernel_stack_top = (void *)((uintptr_t)data->kernel_stack_ptr + kernel::kernel_stack_size);
    data->syscall_stack_top = (void *)((uintptr_t)data->syscall_stack_ptr + kernel::kernel_stack_size - 16);
    data->simd_context = try$(arch::x86_64::SimdContext::create());
    
    data->saved_syscall_stack = 0;

 
    auto frame = arch::amd64::StackFrame();

    frame.rsp = (uint64_t)data->stack_top;
    frame.rbp = (uint64_t)0;
    frame.rip = (uint64_t)launch.entry;
    frame.rdi = launch.args[0];
    frame.rsi = launch.args[1];
    frame.rdx = launch.args[2];
    frame.rcx = launch.args[3];
    frame.r8 = launch.args[4];

    if (launch.user)
    {
        frame.cs = (arch::amd64::Gdt::user_code_segment_id * 8) | 3;
        frame.ss = (arch::amd64::Gdt::user_data_segment_id * 8) | 3;
    }
    else
    {
        frame.cs = arch::amd64::Gdt::kernel_code_segment_id * 8;
        frame.ss = arch::amd64::Gdt::kernel_data_segment_id * 8;
    }

    frame.rflags = arch::amd64::RFLAGS_INTERRUPT_ENABLE | arch::amd64::RFLAGS_ONE;

    data->stackframe(frame);

    return {};
}

core::Result<CpuContext *> CpuContext::create(CpuContextLaunch launch)
{
    auto ctx = try$(create_empty());
    try$(ctx->prepare(launch));
    return {ctx};
}

} // namespace kernel