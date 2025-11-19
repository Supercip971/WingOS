#include <stdint.h>
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

    MachineContextShared shared = {};

    log::log$("framebuffer addr: {}", (uintptr_t)context->_framebuffer.address | fmt::FMT_HEX);
    shared.framebuffer_width = context->_framebuffer.width;
    shared.framebuffer_height = context->_framebuffer.height;
    shared.framebuffer_addr = (uintptr_t)context->_framebuffer.address - 0xffff800000000000;
    startup_init_service(server, shared);

    return 1;
}