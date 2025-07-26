#include "iol/wingos/syscalls.h"
#include "mcx/mcx.hpp"
const char* hello_world = "Hello, World!\n";

extern"C" int _start(mcx::MachineContext* context)
{

    for (int i = 0; i < context->_modules_count; i++)
    {
        sys$debug_log(context->_modules[i].path);
    }
    while(true)
    {
     //   sys$debug_log("Hello, World!");
    }
    return 1;
}