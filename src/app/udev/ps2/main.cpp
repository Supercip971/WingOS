#include <stddef.h>
#include <string.h>

#include "protocols/hi/human_interface.hpp"
#include "protocols/server_helper.hpp"

#include "app/udev/ps2/controller.hpp"
#include "app/udev/ps2/keyboard.hpp"
#include "app/udev/ps2/mouse.hpp"
#include "app/udev/ps2/server.hpp"
#include "iol/wingos/space.hpp"
#include "libcore/ds/vec.hpp"
#include "libcore/fmt/log.hpp"
#include "libcore/type-utils.hpp"
#include "protocols/pipe/pipe.hpp"

// source: derived from brutal OS but
// I wrote the brutal PS2 code

fc::Vec<prot::Duplex<prot::HIEvent> *> mouse_pipes = {};
fc::Vec<prot::Duplex<prot::HIEvent> *> keyboard_pipes = {};

int main(int, char **)
{
    mouse_pipes = {};
    keyboard_pipes = {};
    fmt::log$("Start ps2 app");

    auto server_r = prot::ManagedServer::create_registered_server<Ps2Server>("human-interface", 1, 0);

    if (server_r.is_error())
    {
        fmt::err$("failed to create hio server: {}", server_r.error());
        return -1;
    }

    auto server = std::move(server_r.unwrap());

    fmt::log$("started ps2 service");

    Ps2::Controller controller = Ps2::Controller();
    Ps2::Ps2Keyboard keyboard(controller);
    keyboard.init();
    Ps2::Mouse mouse(controller);
    mouse.init();

    controller.flush();
    while (true)
    {
        server->do_receive_async();

        if (mouse.handle_event())
        {
            auto ev_res = mouse.poll_event();
            while (!ev_res.is_error())
            {
                auto mouse_ev = ev_res.take();
                // fmt::log$("mouse event: dx={} dy={}", mouse_ev.offx, mouse_ev.offy);
                prot::HIEvent event = {};
                event.type = prot::HI_EVENT_TYPE_MOUSE;
                event.mouse.dx = mouse_ev.offx;
                event.mouse.dy = mouse_ev.offy;
                event.mouse.buttons = (mouse_ev.left ? 1 : 0) | (mouse_ev.right ? 2 : 0) | (mouse_ev.middle ? 4 : 0);

                for (auto &conn : server->connected_clients())
                {
                    Ps2Connection *ps = dynamic_cast<Ps2Connection *>(conn.value);
                    if (ps && ps->event_types & prot::HI_EVENT_TYPE_MOUSE)
                    {
                        ps->pipe.ring->produce(event);
                    }
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
                fmt::log$("keyboard event: key={} down={}", kb_ev.key, kb_ev.down);
                prot::HIEvent event = {};
                event.type = prot::HI_EVENT_TYPE_KEYBOARD;
                event.keyboard.keycode = kb_ev.key;
                event.keyboard.pressed = kb_ev.down;
                for (auto &conn : server->connected_clients())
                {
                    Ps2Connection *ps = dynamic_cast<Ps2Connection *>(conn.value);
                    if (ps && ps->event_types & prot::HI_EVENT_TYPE_KEYBOARD)
                    {
                        ps->pipe.ring->produce(event);
                    }
                }
                ev_res = keyboard.poll_event();
            }
        }
    }
}
