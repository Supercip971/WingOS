#include <stdint.h>
#include <stdlib.h>

#include "app/init/module_startup.hpp"
#include "app/init/service_register.hpp"
#include "protocols/server_helper.hpp"

#include "iol/wingos/space.hpp"
#include "libcore/fmt/flags.hpp"
#include "libcore/fmt/log.hpp"
#include "mcx/mcx.hpp"
#include "wingos-headers/startup.hpp"

int main(int, char **)
{
    while (true)
    {
    }
};

int _main(StartupInfo *info)
{
    mcx::MachineContext *context = &info->machine_context_optional;

    fmt::log$("hello world from init!");

    auto server = prot::ManagedServer::create_server<InitIpcServer>(true).take();
    fmt::log$("created server with handle: {}", server.addr());

    startup_module(context);

    MachineContextShared shared = {};

    fmt::log$("framebuffer addr: {}", (uintptr_t)context->_framebuffer.address | fmt::FMT_HEX);
    shared.framebuffer_width = context->_framebuffer.width;
    shared.framebuffer_height = context->_framebuffer.height;
    shared.framebuffer_addr = (uintptr_t)context->_framebuffer.address - 0xffff800000000000;
    server.mcx_shared(shared);
    server.loop();

    return 1;
}
