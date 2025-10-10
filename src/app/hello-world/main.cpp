#include "arch/generic/syscalls.h"
#include "iol/wingos/space.hpp"
#include "protocols/init/init.hpp"


#include "iol/wingos/syscalls.h"
#include "libcore/fmt/log.hpp"
#include "mcx/mcx.hpp"
#include "wingos-headers/syscalls.h"

int _main(mcx::MachineContext* )
{

    // attempt connection to server ID 0
  

    auto conn = prot::InitConnection::connect().unwrap();
 
    

    log::log$("(client) connected to server with handle: {}", conn.raw_client().handle );


   // now do a call

    prot::InitRegisterServer reg = {};
    core::Str("hello-world").copy_to((char*)reg.name, 80);

    reg.major = 1;
    reg.minor = 0;
    reg.endpoint = 42;

    conn.register_server(reg).unwrap();

    prot::InitGetServer get = {

        .name = {},
        .major = 1,
        .minor = 0,
    }; 

    core::Str("hello-world").copy_to((char*)get.name, 80);
    
    auto g = conn.get_server(get).unwrap();
    log::log$("(client) got server endpoint: {}, version: {}.{}", g.endpoint, g.major, g.minor);
    while (true)
    {
    }

    log::log$("dead...");
    while (true)
    {
    }
    return 1;
}