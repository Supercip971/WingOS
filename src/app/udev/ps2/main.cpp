#include <stddef.h>
#include <string.h>
#include <sys/types.h>

#include "libcore/fmt/fmt_str.hpp"
#include "libcore/str_writer.hpp"
#include "protocols/hi/human_interface.hpp"
#include "protocols/server_helper.hpp"

#include "app/udev/ps2/controller.hpp"
#include "app/udev/ps2/keyboard.hpp"
#include "app/udev/ps2/mouse.hpp"
#include "arch/generic/syscalls.h"
#include "iol/ports.hpp"
#include "iol/wingos/execute.hpp"
#include "iol/wingos/space.hpp"
#include "iol/wingos/syscalls.h"
#include "libcore/alive.hpp"
#include "libcore/fmt/log.hpp"
#include "mcx/mcx.hpp"
#include "protocols/compositor/compositor.hpp"
#include "protocols/compositor/window.hpp"
#include "protocols/init/init.hpp"
#include "protocols/pipe/pipe.hpp"
#include "protocols/vfs/vfs.hpp"
#include "wingos-headers/startup.hpp"
#include "wingos-headers/syscalls.h"

// source: derived from brutal OS but
// I wrote the brutal PS2 code

static core::Vec<prot::SenderPipe*> mouse_pipes = {};
static core::Vec<prot::SenderPipe*> keyboard_pipes = {};

int main(int, char **)
{
    mouse_pipes = {};
    keyboard_pipes = {};

    log::log$("Start ps2 app");
    auto server_r = prot::ManagedServer::create_registered_server("human-interface", 1, 0);

    if(server_r.is_error())
    {
        log::err$("failed to create hio server: {}", server_r.error());
        return -1;
    }

    prot::ManagedServer server = core::move(server_r.unwrap());

    log::log$("started ps2 service");

    Ps2::Controller controller = Ps2::Controller();
    Ps2::Ps2Keyboard keyboard(controller);
    keyboard.init();
    Ps2::Mouse mouse(controller);
    mouse.init();

    controller.flush();
    while (true)
    {

        server.accept_connection();


        auto received = server.try_receive();
        if (!received.is_error())
        {
            auto msg = core::move(received.unwrap());

            switch (msg.received.data[0].data)
            {
            case prot::HI_START_LISTEN:
            {
                uint32_t event_types = (uint32_t)msg.received.data[1].data;


                auto pipe = prot::Duplex::create(Wingos::Space::self(), Wingos::Space::self() );

                if (pipe.is_error())
                {
                    log::err$("hio: failed to create duplex pipe: {}", pipe.error());
                    break;
                }

                auto duplex = core::move(pipe.unwrap());

                log::log$("hio: duplex handles: {} {}", duplex.connection_sender.handle, duplex.connection_receiver.handle);
                prot::SenderPipe* sender = new prot::SenderPipe(core::move(duplex.connection_sender));

                if (event_types & prot::HI_EVENT_TYPE_MOUSE)
                {
                    mouse_pipes.push(sender);
                    log::log$("hio: added mouse pipe: {}", sender->raw_connection().handle);

                }

                if (event_types & prot::HI_EVENT_TYPE_KEYBOARD)
                {
                    keyboard_pipes.push(sender);
                    log::log$("hio: added keyboard pipe");
                }

                IpcMessage resp = {};
                resp.data[0].asset_handle = duplex.connection_receiver.handle;
                log::log$("hio: replying with receiver handle: {}", resp.data[0].asset_handle);
                resp.data[0].is_asset = true;
                server.reply(core::move(msg), resp).unwrap();

                break;
            }
            default:
                log::warn$("hio: unknown message type received: {}", msg.received.data[0].data);
                break;
            }
        }

        if (mouse.handle_event())
        {
            auto ev_res = mouse.poll_event();
            while (!ev_res.is_error())
            {
                auto mouse_ev = ev_res.take();
                log::log$("mouse event: dx={} dy={}", mouse_ev.offx, mouse_ev.offy);
                prot::HIEvent event = {};
                event.type = prot::HI_EVENT_TYPE_MOUSE;
                event.mouse.dx = mouse_ev.offx;
                event.mouse.dy = mouse_ev.offy;
                event.mouse.buttons = (mouse_ev.left ? 1 : 0) | (mouse_ev.right ? 2 : 0) | (mouse_ev.middle ? 4 : 0);

                for (size_t i = 0; i < mouse_pipes.len(); i++)
                {
                    mouse_pipes[i]->send(&event, sizeof(event));
                }
                ev_res = mouse.poll_event();
            }
        }

        if (keyboard.packet_handle())
        {
            auto ev_res = keyboard.poll_event();
            while (!ev_res.is_error())
            {
                auto kb_ev = ev_res.take();
                log::log$("keyboard event: key={} down={}", kb_ev.key, kb_ev.down);
                prot::HIEvent event = {};
                event.type = prot::HI_EVENT_TYPE_KEYBOARD;
                event.keyboard.keycode = kb_ev.key;
                event.keyboard.pressed = kb_ev.down;
                for (size_t i = 0; i < keyboard_pipes.len(); i++)
                {
                    keyboard_pipes[i]->send(&event, sizeof(event));
                }
                ev_res = keyboard.poll_event();
            }
        }
    }
}
