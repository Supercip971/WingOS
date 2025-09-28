#include <stdlib.h>

#include "app/init/module_startup.hpp"
#include "app/init/service_register.hpp"

#include "mcx/mcx.hpp"

int _main(mcx::MachineContext *context)
{
    log::log$("hello world from init!");

    auto server = Wingos::Space::self().create_ipc_server(true);
    log::log$("created server with handle: {}", server.handle);

    startup_module(context);

    startup_init_service(server);

    return 1;
}