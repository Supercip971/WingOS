
#include <arch/generic/instruction.hpp>

void arch::pause()
{
    asm volatile("pause");
}

void arch::invalidate(void* addr)
{
    asm volatile("clflush (%0)" ::"r"(addr) : "memory");
}