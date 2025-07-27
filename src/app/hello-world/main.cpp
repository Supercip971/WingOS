#include "iol/wingos/syscalls.h"
#include "mcx/mcx.hpp"

extern "C" int _start(void)
{

    while (true)
    {
        sys$debug_log("Hello, World!");
    }
    return 1;
}