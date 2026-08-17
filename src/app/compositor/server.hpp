#pragma once

#include <stddef.h>
#include <string.h>

#include "protocols/server_helper.hpp"

#include "iol/wingos/asset.hpp"
#include "protocols/compositor/window.hpp"
#include "protocols/init/init.hpp"

struct Window : public prot::ManagedServerConnectionHandler
{

    size_t width;
    size_t height;
    bool is_framebuffer_taken;

    Wingos::MemoryAsset framebuffer_asset;
    int depth;

    Wingos::VirtualMemoryAsset framebuffer_mapped;
    void *screen_buffer;

    virtual bool init() { return true; }

    virtual void signal_disconnect(IpcMessage &msg)
    {
        (void)msg;
    };

    // return true on reply
    virtual fc::Result<void> call_received(IpcMessage &msg, fc::Optional<Wingos::IpcReplyObject> reply_obj [[maybe_unused]])
    {

        switch (msg.arg(0))
        {
        case prot::COMPOSITOR_CREATE_WINDOW:
        {
            ack(reply_obj);
            return {};
        }

        case prot::WINDOW_GET_ATTRIBUTE_SIZE:
        {
            prot::WindowGetAttributeSize resp{};
            resp.width = width;
            resp.height = height;

            IpcMessage reply = {};
            reply.arg(0, resp.width);
            reply.arg(1, resp.height);
            ret(reply);

            break;
        }
        case prot::WINDOW_GET_FRAMEBUFFER:
        {
            IpcMessage reply = {};
            reply.copy_handle(0, framebuffer_asset.handle);
            ret(reply);
            break;
        }
        case prot::WINDOW_SWAP_BUFFERS:
        {
            IpcMessage reply = {}; // ack message
            memcpy((void *)screen_buffer, (void *)framebuffer_mapped.ptr(), width * height * 4);
            ret(reply);
            break;
        }
        default:
        {
            fmt::warn$("compositor: unknown window message type received: {}", msg.arg(0));
            break;
        }
        }

        return {};
    }
};

class CompositorServer : public prot::ManagedServer
{

public:
    size_t cdepth = 0;
    prot::InitQueryFbResponse fb;
    void *mapped_fb;

    CompositorServer()
    {
    }

    virtual fc::Result<prot::ManagedServerConnectionHandler *> on_connect(IpcMessage &initiator) final
    {
        switch (initiator.arg(0))
        {

        case prot::COMPOSITOR_CREATE_WINDOW:
        {
            bool take_fb = initiator.arg(1) != 0;

            auto wnd = new Window();

            auto window_mem = Wingos::Space::self().allocate_physical_memory(fb.framebuffer_width * fb.framebuffer_height * 4);
            auto window_map = Wingos::Space::self().map_memory(window_mem, ASSET_MAPPING_FLAG_READ | ASSET_MAPPING_FLAG_WRITE);

            wnd->width = fb.framebuffer_width;
            wnd->height = fb.framebuffer_height;
            wnd->is_framebuffer_taken = take_fb;
            wnd->framebuffer_asset = window_mem;
            wnd->depth = cdepth++;
            wnd->framebuffer_mapped = window_map;
            wnd->screen_buffer = (void *)mapped_fb;

            return wnd;

            // case prot::VFS_ACCESS_PWD:
            // unimplemented
        }

        default:
            fmt::log$("invalid connect access for compositor: {}", initiator.arg(0));
            return "invalid connect access for compositor";
        }

        return "invalid connect access for compositor";
    }

    virtual fc::Result<void> after_receive() final
    {
        return {};
    }

    virtual ~CompositorServer() {}
};
