#pragma once

#include "arch/x86_64/context.hpp"

#include "kernel/generic/context.hpp"
#include "libcore/atomic.hpp"
namespace arch::amd64
{

class CpuContextAmd64 : public kernel::CpuContext
{
public:
    CpuContextAmd64() = default;

    StackFrame frame;

    void _user_stack_addr(uintptr_t addr)
    {
        frame.rsp = addr;
    }

    void stackframe(StackFrame nframe)
    {
        frame = nframe;
    }

    StackFrame stackframe()  const
    {
        return frame;
    }
};
} // namespace arch::amd64