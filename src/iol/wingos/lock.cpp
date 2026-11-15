#include "iol/lock_context.h"

int enter_critical_context()
{
    return 0;
}

void exit_critical_context(int previous_state)
{
    (void)previous_state;
}
