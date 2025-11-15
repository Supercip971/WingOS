#pragma once

#include <mcx/mcx.hpp>

void arch_entry(const mcx::MachineContext *context);

void kernel_entry(const mcx::MachineContext *context);

namespace kernel
{
constexpr uintptr_t userspace_stack_base = (0xcc0000000);
constexpr size_t kernel_stack_size = (16384);
constexpr size_t userspace_stack_size = (16384);

} // namespace kernel