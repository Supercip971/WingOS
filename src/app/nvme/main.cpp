#include "arch/generic/syscalls.h"
#include "iol/wingos/space.hpp"
#include "iol/wingos/syscalls.h"
#include "libcore/fmt/log.hpp"
#include "mcx/mcx.hpp"
#include "wingos-headers/syscalls.h"

int _main(mcx::MachineContext*)
{

    // attempt connection 

    log::log$("hello world from nvme!");

    
    while(true)
    {

    }
    return 1;
}