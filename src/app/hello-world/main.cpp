#include "iol/wingos/syscalls.h"
#include "libcore/fmt/log.hpp"
#include "mcx/mcx.hpp"

int _main(mcx::MachineContext*)
{

    while (true)
    {
        log::log$("Hello, World!");

    }
    return 1;
}