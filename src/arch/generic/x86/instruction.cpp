
#include <arch/generic/instruction.hpp>

void arch::pause()
{
    asm volatile("pause");
}