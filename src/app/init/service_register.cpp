#include "service_register.hpp"
#include <stdlib.h>

#include "iol/wingos/ipc.hpp"
#include "iol/wingos/space.hpp"
#include "libcore/fmt/log.hpp"
#include "wingos-headers/ipc.h"

void startup_init_service(Wingos::IpcServer server)
{

    log::log$("created server with handle: {}", server.handle);
    while (true)
    {

        auto conn = server.accept();
        if (!conn.is_error())
        {
            log::log$("(server) accepted connection: {}", conn.unwrap()->handle);
        }

        auto received = server.receive();

        if (!received.is_error())
        {
            auto msg = received.unwrap();
            log::log$("(server) received message: {}", msg.received.data[0].data);

            IpcMessage reply = {};
            reply.data[0].data = 1234;
            reply.data[0].is_asset = false;

            server.reply(core::move(msg), reply).assert();
        }
    }
}