#include "protocols/server_helper.hpp"
#include <string.h>

#include "arch/generic/syscalls.h"
#include "iol/wingos/asset.hpp"
#include "iol/wingos/ipc.hpp"
#include "iol/wingos/space.hpp"
#include "iol/wingos/syscalls.h"
#include "libcore/alive.hpp"
#include "libcore/ds/vec.hpp"
#include "libcore/fmt/log.hpp"
#include "mcx/mcx.hpp"
#include "protocols/compositor/compositor.hpp"
#include "protocols/compositor/window.hpp"
#include "protocols/init/init.hpp"
#include "protocols/vfs/vfs.hpp"
#include "wingos-headers/ipc.h"
#include "wingos-headers/syscalls.h"


struct FramebufferInfo 
{
    uintptr_t framebuffer_addr;
    size_t framebuffer_width;
    size_t framebuffer_height;
};


int cdepth = 0;

int mdepth = 0; // manager depth 

struct Window 
{
    prot::ManagedServer server;

    size_t width; 
    size_t height; 
    bool is_framebuffer_taken;

    Wingos::MemoryAsset framebuffer_asset;
    int depth; 

    Wingos::VirtualMemoryAsset framebuffer_mapped;

};
core::Vec<Window> windows = {};

void * framebuffer_mapped = nullptr;

bool update_window(Window &window) 
{

    if(window.server.connection_count() == 0) 
    {

        window.server.accept_connection();
        return false;
    }

    core::Vec<Wingos::MessageServerReceived> msgs = {};

    auto received = window.server.try_receive();

    if (received.is_error()) 
    {
        return false;
    }

    auto msg = received.take();

    if (msg.received.flags & IPC_MESSAGE_FLAG_DISCONNECT) 
    {
        log::log$("compositor: disconnecting window");
        window.server.raw_server().disconnect(msg.connection);
        return true;
    }
 
    switch (msg.received.data[0].data) 
    {
        case prot::WINDOW_GET_ATTRIBUTE_SIZE: 
        {
            prot::WindowGetAttributeSize resp{};
            resp.width = window.width;
            resp.height = window.height;

            IpcMessage reply = {};
            reply.data[0].data = resp.width;
            reply.data[1].data = resp.height;

            window.server.reply(core::move(msg), reply).unwrap();
            break;
        }
        case prot::WINDOW_GET_FRAMEBUFFER: 
        {
            IpcMessage reply = {};
            auto fb_asset =  window.framebuffer_asset;
            reply.data[0].data = fb_asset.handle;
            reply.data[0].is_asset = true;
            reply.data[0].copy_asset = true;
            window.server.reply(core::move(msg), reply).unwrap();
            break;
        }
        case prot::WINDOW_SWAP_BUFFERS:
        {
            IpcMessage reply = {}; // ack message
            memcpy((void *)framebuffer_mapped, (void *)window.framebuffer_mapped.ptr(), window.width * window.height * 4);
            window.server.reply(core::move(msg), reply).unwrap();
            break;
        }
        default:
        {
            log::warn$("compositor: unknown window message type received: {}", msg.received.data[0].data);
            break;
        }
    }

    return false;
}
int main(int, char**)
{
    core::Alive alive {"compositor"};

    auto serv_g = prot::ManagedServer::create_registered_server("compositor", 1, 0);

    auto init_info = prot::InitConnection::connect().unwrap();

    auto fb = init_info.query_framebuffer().unwrap();

    // attempt connection to open root file
    auto serv = serv_g.take();


    auto framebuffer_phys = Wingos::Space::self().own_memory_physical(fb.framebuffer_addr, fb.framebuffer_width * fb.framebuffer_height * 4);
    auto framebuffer_map = Wingos::Space::self().map_memory(framebuffer_phys, ASSET_MAPPING_FLAG_READ | ASSET_MAPPING_FLAG_WRITE);
    
    framebuffer_mapped = framebuffer_map.ptr();


    for(size_t i = 0; i < fb.framebuffer_width * fb.framebuffer_height * 4; i++) 
    {
        ((uint8_t *)framebuffer_mapped)[i] = 0xff; // white
    }
    while (true)
    {
       // alive.tick();
        size_t idx = 0;
        for(auto &window : windows) 
        {
            if(update_window(window)) 
            {
                // window closed
                windows.pop(idx);
                break;
            }
            idx++;
        }


        serv.accept_connection();

        auto received = serv.try_receive();
        if (received.is_error())
        {
            continue;
        }
        auto msg = core::move(received.unwrap());

        if (msg.received.flags & IPC_MESSAGE_FLAG_DISCONNECT)
        {
            log::log$("compositor: disconnecting from client");
            serv.raw_server().disconnect(msg.connection);
            continue;
        }

        

        switch (msg.received.data[0].data)
        {


        case prot::COMPOSITOR_CREATE_WINDOW:
        {
            bool take_fb = msg.received.data[1].data != 0;
            prot::CompositorCreateWindow resp{};
            auto window_conn_r = prot::ManagedServer::create_server();
            if (window_conn_r.is_error())
            {
                log::err$("compositor: failed to create window server: {}", window_conn_r.error());
                resp.window_endpoint = 0;
            }
            else
            {
                auto window_conn = core::move(window_conn_r.unwrap());
                resp.window_endpoint = window_conn.addr();

                auto window_mem = Wingos::Space::self().allocate_physical_memory(fb.framebuffer_width * fb.framebuffer_height * 4);
                auto window_map = Wingos::Space::self().map_memory(window_mem, ASSET_MAPPING_FLAG_READ | ASSET_MAPPING_FLAG_WRITE); 

                mdepth = cdepth+1;
                windows.push(Window{
                    .server = core::move(window_conn),
                    .width = fb.framebuffer_width,
                    .height = fb.framebuffer_height,
                    .is_framebuffer_taken = take_fb,
                    .framebuffer_asset = window_mem,
                    .depth = cdepth++,
                    .framebuffer_mapped = window_map,
                });
                
                log::log$("compositor: created window with endpoint {}, take_fb={}", resp.window_endpoint, take_fb);
            }

            IpcMessage reply = {};
            reply.data[0].data = resp.window_endpoint;
            serv.reply(core::move(msg), reply).unwrap();


            break;
        }
        
        default:
        {
            log::warn$("compositor: unknown message type received: {}", msg.received.data[0].data);
            
            break;
        }

        }
    }
}