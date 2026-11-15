#include <stddef.h>
#include <string.h>
#include <utility>

#include "protocols/hi/human_interface.hpp"
#include "protocols/server_helper.hpp"

#include "app/udev/ps2/controller.hpp"
#include "app/udev/ps2/keyboard.hpp"
#include "app/udev/ps2/mouse.hpp"
#include "app/udev/ps2/server.hpp"
#include "iol/wingos/asset.hpp"
#include "iol/wingos/space.hpp"
#include "iol/wingos/syscalls.h"
#include "libcore/ds/vec.hpp"
#include "libcore/fmt/log.hpp"
#include "protocols/pipe/pipe.hpp"

// source: derived from brutal OS but
// I wrote the brutal PS2 code

static fc::Vec<prot::Duplex<prot::HIEvent> *> mouse_pipes = {};
static fc::Vec<prot::Duplex<prot::HIEvent> *> keyboard_pipes = {};
static Wingos::SignalAsset mouse_signals = {};
static Wingos::SignalAsset keyboard_signals = {};

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

    mouse_signals = Wingos::Space::self().create_signal_endpoint();
    keyboard_signals = Wingos::Space::self().create_signal_endpoint();
    server->init(&mouse_signals, &keyboard_signals);
    Ps2::Controller controller = Ps2::Controller();
    Ps2::Ps2Keyboard keyboard(controller);
    keyboard.init();
    Ps2::Mouse mouse(controller);
    mouse.init();

    controller.flush();

    auto int12 = sys$ipc_wait_for_interrupt_async(12);
    auto int1 = sys$ipc_wait_for_interrupt_async(1);

    fmt::log$("started ps2 service");
    while (true)
    {
        if (mouse.handle_event())
        {
            auto ev_res = mouse.poll_event();
            while (!ev_res.is_error())
            {
                auto mouse_ev = ev_res.take();
                fmt::log$("mouse event: dx={} dy={}", mouse_ev.offx, mouse_ev.offy);
                prot::HIEvent event = {};
                event.type = prot::HI_EVENT_TYPE_MOUSE;
                event.mouse.dx = mouse_ev.offx;
                event.mouse.dy = mouse_ev.offy;
                event.mouse.buttons = (mouse_ev.left ? 1 : 0) | (mouse_ev.right ? 2 : 0) | (mouse_ev.middle ? 4 : 0);

                for (auto &conn : server->connected_clients())
                {
                    auto *ps = dynamic_cast<Ps2Connection *>(conn.value);
                    if (ps && ps->event_types & prot::HI_EVENT_TYPE_MOUSE)
                    {
                        ps->pipe.ring->produce(event);
                    }
                }

                ev_res = mouse.poll_event();
            }
            mouse_signals.ping();
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
                    auto *ps = dynamic_cast<Ps2Connection *>(conn.value);
                    if (ps && ps->event_types & prot::HI_EVENT_TYPE_KEYBOARD)
                    {
                        ps->pipe.ring->produce(event);
                    }
                }
                ev_res = keyboard.poll_event();
            }
            keyboard_signals.ping();
        }

        auto interrupted_by = server->do_receive();
        if (interrupted_by.is_ok() && interrupted_by.unwrap() != 0)
        {
            if (interrupted_by.unwrap() == int1.return_asset_id)
            {
                fmt::log$("keyboard int");
            }
            else if (interrupted_by.unwrap() == int12.return_asset_id)
            {
                fmt::log$("mouse int");
            }
        }
    }
}
