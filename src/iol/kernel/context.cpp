

#include "arch/x86_64/interrupts.hpp"
int enter_critical_context()
{

    if(arch::amd64::interrupt_status())
    {
        arch::amd64::interrupt_hold();
        return 1;
    }
    return 0;
}
void exit_critical_context(int previous_state)
{
    if(previous_state)
    {
        arch::amd64::interrupt_release();
    }


}
