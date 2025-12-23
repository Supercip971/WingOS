#include <mcx/mcx.hpp>
#include "kernel/generic/kernel.hpp"

void arch_entry(const mcx::MachineContext *context)
{
    kernel_entry(context);
   (void)context;

}
