#include <string.h>

#include "protocols/hi/human_interface.hpp"
#include "protocols/server_helper.hpp"

#include "app/compositor/server.hpp"
#include "iol/wingos/asset.hpp"
#include "iol/wingos/ipc.hpp"
#include "iol/wingos/space.hpp"
#include "libcore/alive.hpp"
#include "libcore/ds/vec.hpp"
#include "libcore/fmt/log.hpp"
#include "libcore/type-utils.hpp"
#include "protocols/compositor/window.hpp"
#include "protocols/init/init.hpp"
#include "wingos-headers/asset.h"
#include "wingos-headers/ipc.h"

struct FramebufferInfo
{
    uintptr_t framebuffer_addr;
    size_t framebuffer_width;
    size_t framebuffer_height;
};

int mdepth = 0; // manager depth

void *framebuffer_mapped = nullptr;

static prot::HIConnection *conn_inputs;

int main(int, char **)
{
    fc::Alive alive{"compositor"};

    auto serv_g = prot::ManagedServer::create_registered_server<CompositorServer>("compositor", 1, 0);

    auto init_info = prot::InitConnection::connect().unwrap();

    auto fb = init_info.query_framebuffer().unwrap();

    // attempt connection to open root file
    auto serv = serv_g.take();

    auto framebuffer_phys = Wingos::Space::self().own_memory_physical(fb.framebuffer_addr, fb.framebuffer_width * fb.framebuffer_height * 4);
    auto framebuffer_map = Wingos::Space::self().map_memory(framebuffer_phys, ASSET_MAPPING_FLAG_READ | ASSET_MAPPING_FLAG_WRITE);

    framebuffer_mapped = framebuffer_map.ptr();

    for (size_t i = 0; i < fb.framebuffer_width * fb.framebuffer_height * 4; i++)
    {
        ((uint8_t *)framebuffer_mapped)[i] = 0xff; // white
    }

    serv->fb = fb;
    serv->mapped_fb = framebuffer_mapped;

    conn_inputs = new prot::HIConnection(prot::HIConnection::connect().take());
    conn_inputs->start_listen().take();
    auto res = conn_inputs->enable_signaling().take();

    auto interruption_id = res.enable_interruption();

    while (true)
    {
        // alive.tick();
        auto interrupted_by = serv->do_receive();

        if (interrupted_by.is_ok() && interrupted_by.unwrap() != 0)
        {
            if (interrupted_by.unwrap() == interruption_id.handle)
            {
                conn_inputs->event_queue().update_event();

                while (true)
                {
                    auto ev_res = conn_inputs->event_queue().poll_event();
                    if (ev_res.is_error())
                    {
                        break;
                    }
                    auto ev = ev_res.unwrap();
                    serv->handle_input_event(ev);
                    fmt::log$("input event: type={}, dx={}, dy={}, keycode={}", (int)ev.type, ev.mouse.dx, ev.mouse.dy, ev.keyboard.keycode);
                }
            }
            else
            {
                fmt::err$("Compositor error: interrupted by: {}", interrupted_by.unwrap());
            }
        }
    }
    return 0;
}
