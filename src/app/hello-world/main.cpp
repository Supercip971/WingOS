#include "arch/generic/syscalls.h"
#include "iol/wingos/syscalls.h"
#include "libcore/fmt/log.hpp"
#include "mcx/mcx.hpp"
#include "wingos-headers/syscalls.h"

int _main(mcx::MachineContext*)
{

    // attempt connection to server ID 0

    SyscallIpcConnect connect = sys$ipc_connect(0, SPACE_SELF, 0, 0);
    (void)connect;

    if(connect.returned_handle == 0)
    {
        log::log$("no connection available");
    }
    else
    {
        log::log$("connected to server with handle: {}", connect.returned_handle);
    }
    while (true)
    {
      //  log::log$("Hello, World!");
    }
    return 1;
}