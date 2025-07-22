#include "iol/wingos/syscalls.h"
const char* hello_world = "Hello, World!\n";

extern"C" int _start()
{
    while(true)
    {
        sys$debug_log("Hello, World!");
    }
    return 1;
}