#include <string.h>

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
    while (true)
    {
        // alive.tick();
        serv->loop();
    }
    return 0;
}
