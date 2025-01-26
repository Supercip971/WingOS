#include <kernel/generic/task.hpp>
#include <libcore/fmt/log.hpp>

#include "arch/x86_64/gdt.hpp"
#include "kernel/x86_64/context.hpp"
#include "kernel/x86_64/cpu.hpp"

#include "kernel/generic/context.hpp"
#include "kernel/generic/kernel.hpp"
#include "libcore/alloc/alloc.hpp"

namespace kernel
{
core::Result<CpuContext *> CpuContext::create_empty()
{

    arch::amd64::CpuContextAmd64 *data = try$(core::mem_alloc<arch::amd64::CpuContextAmd64>());

    return data;
}

void CpuContext::load_to(void *state) const
{
    arch::amd64::CpuContextAmd64 const *data = this->as<arch::amd64::CpuContextAmd64>();

    arch::amd64::StackFrame *frame = (arch::amd64::StackFrame *)state;
    *frame = data->frame;
}

void CpuContext::save_in(void *state)
{
    arch::amd64::CpuContextAmd64 *data = this->as<arch::amd64::CpuContextAmd64>();

    arch::amd64::StackFrame *frame = (arch::amd64::StackFrame *)state;
    data->frame = *frame;
}

void CpuContext::release()
{
    arch::amd64::CpuContextAmd64 *data = this->as<arch::amd64::CpuContextAmd64>();

    if (data->stack_ptr != nullptr)
        core::mem_free(data->stack_ptr);
    if (data->kernel_stack_ptr != nullptr)
        core::mem_free(data->kernel_stack_ptr);

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

    data->stack_top = (void *)((uintptr_t)data->stack_ptr + kernel::userspace_stack_size);
    data->kernel_stack_top = (void *)((uintptr_t)data->kernel_stack_ptr + kernel::kernel_stack_size);

    data->frame = arch::amd64::StackFrame();

    data->frame.rsp = (uint64_t)data->stack_top;
    data->frame.rbp = (uint64_t)data->stack_top;
    data->frame.rip = (uint64_t)launch.entry;
    data->frame.rdi = launch.args[0];
    data->frame.rsi = launch.args[1];
    data->frame.rdx = launch.args[2];
    data->frame.rcx = launch.args[3];
    data->frame.r8 = launch.args[4];

    if (launch.user)
    {
        data->frame.cs = (arch::amd64::Gdt::user_code_segment_id * 8) | 3;
        data->frame.ss = (arch::amd64::Gdt::user_data_segment_id * 8) | 3;
    }
    else
    {
        data->frame.cs = arch::amd64::Gdt::kernel_code_segment_id * 8;
        data->frame.ss = arch::amd64::Gdt::kernel_data_segment_id * 8;
    }

    data->frame.rflags = arch::amd64::RFLAGS_INTERRUPT_ENABLE | arch::amd64::RFLAGS_ONE;

    return {};
}

core::Result<CpuContext *> CpuContext::create(CpuContextLaunch launch)
{
    auto ctx = try$(create_empty());
    try$(ctx->prepare(launch));
    return {ctx};
}

} // namespace kernel