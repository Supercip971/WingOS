#pragma once

#include <cstdint>
#include <stddef.h>
#include <string.h>

#include "protocols/hi/human_interface.hpp"
#include "protocols/server_helper.hpp"

#include "cursor.hpp"
#include "gfx/color.hpp"
#include "gfx/geometry/rect.hpp"
#include "iol/wingos/asset.hpp"
#include "iol/wingos/ipc.hpp"
#include "libcore/fmt/log.hpp"
#include "libcore/logic.hpp"
#include "libcore/optional.hpp"
#include "libcore/result.hpp"
#include "protocols/compositor/window.hpp"
#include "protocols/init/init.hpp"
#include "wingos-headers/asset.h"
#include "wingos-headers/ipc.h"
class CompositorServer;

struct Window : public prot::ManagedServerConnectionHandler
{

    size_t width;
    size_t height;
    bool is_framebuffer_taken;
    CompositorServer *parent;

    Wingos::MemoryAsset framebuffer_asset;
    int depth;

    Wingos::VirtualMemoryAsset framebuffer_mapped;
    void *screen_buffer;

    virtual bool init() { return true; }

    virtual void signal_disconnect(IpcMessage &msg)
    {
        (void)msg;
    };

    void after_swap(wgfx::GRect const &rendered);

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
            after_swap({});
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
    long cursor_x = 0;
    long cursor_y = 0;
    Window *main_window = nullptr;

    CompositorServer()
    {
    }

    void swapped_window(Window *wnd, wgfx::GRect const &rendered)
    {
        (void)rendered;
        (void)wnd;

        memcpy((void *)mapped_fb, (void *)wnd->framebuffer_mapped.ptr(), wnd->width * wnd->height * 4);
        // render cursor
        wgfx::Rgba8 *fb_ptr = (wgfx::Rgba8 *)mapped_fb;
        for (size_t cx = 0; cx < ICONS8_CURSOR_32_WIDTH; cx++)
        {
            for (size_t cy = 0; cy < ICONS8_CURSOR_32_HEIGHT; cy++)
            {
                long fb_x = cursor_x + cx;
                long fb_y = cursor_y + cy;
                if (fb_x < 0 || fb_x >= (long)fb.framebuffer_width || fb_y < 0 || fb_y >= (long)fb.framebuffer_height)
                    continue;

                wgfx::Rgba8 *pixel = &fb_ptr[fb_x + fb_y * fb.framebuffer_width];
                wgfx::Rgba8 cursor_pixel = wgfx::Rgba8::from_u32rgba(icons8_cursor_32[cy * ICONS8_CURSOR_32_WIDTH + cx]);
                cursor_pixel.r = 0xff;
                cursor_pixel.g = 0xff;
                cursor_pixel.b = 0xff;
                pixel->blend(cursor_pixel);
            }
        }
    }

    void handle_input_event(prot::HIEvent &ev)
    {
        if (ev.type == prot::EventType::HI_EVENT_TYPE_MOUSE)
        {
            cursor_x += ev.mouse.dx;
            cursor_y -= ev.mouse.dy;
            cursor_x = fc::clamp(cursor_x, 0l, (long)fb.framebuffer_width - 1);
            cursor_y = fc::clamp(cursor_y, 0l, (long)fb.framebuffer_height - 1);
            if (main_window != nullptr)
            {
                fmt::log$("swapping window: {} {}", cursor_x, cursor_y);

                swapped_window(main_window, {});
            }
        }
        else if (ev.type == prot::EventType::HI_EVENT_TYPE_KEYBOARD)
        {
        }
    }

    virtual fc::Result<prot::ManagedServerConnectionHandler *> on_connect(IpcMessage &initiator) final
    {
        switch (initiator.arg(0))
        {

        case prot::COMPOSITOR_CREATE_WINDOW:
        {
            bool take_fb = initiator.arg(1) != 0;

            auto wnd = new Window();
            main_window = wnd;

            auto window_mem = Wingos::Space::self().allocate_physical_memory(fb.framebuffer_width * fb.framebuffer_height * 4);
            auto window_map = Wingos::Space::self().map_memory(window_mem, ASSET_MAPPING_FLAG_READ | ASSET_MAPPING_FLAG_WRITE);

            wnd->width = fb.framebuffer_width;
            wnd->height = fb.framebuffer_height;
            wnd->is_framebuffer_taken = take_fb;
            wnd->framebuffer_asset = window_mem;
            wnd->depth = cdepth++;
            wnd->framebuffer_mapped = window_map;
            wnd->screen_buffer = (void *)mapped_fb;
            wnd->parent = this;

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

inline void Window::after_swap(wgfx::GRect const &rendered)
{
    parent->swapped_window(this, rendered);
}
