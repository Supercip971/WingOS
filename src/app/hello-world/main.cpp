#include "arch/generic/syscalls.h"
#include "iol/wingos/space.hpp"
#include "protocols/init/init.hpp"


#include "iol/wingos/syscalls.h"
#include "libcore/fmt/log.hpp"
#include "mcx/mcx.hpp"
#include "wingos-headers/syscalls.h"
#include "protocols/vfs/vfs.hpp"

int _main(mcx::MachineContext* )
{

    // attempt connection to open root file

    auto conn = prot::VfsConnection::connect().unwrap();
    
    auto fd = conn.open_root().unwrap();

    (void)fd;

    log::log$("hello world from vfs app!");

   while (true)
    {
    }
}