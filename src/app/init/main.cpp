#include "iol/wingos/syscalls.h"
#include "libcore/fmt/log.hpp"
#include "mcx/mcx.hpp"
const char* hello_world = "Hello, World!\n";

int _main(mcx::MachineContext* context)
{

    for (int i = 0; i < context->_modules_count; i++)
    {
        log::log$("module {}: {}",i, context->_modules[i].path);
    }
    while(true)
    {
     //   sys$debug_log("Hello, World!");
    }
    return 1;
}