#include "protocols/hi/human_interface.hpp"

#include "gfx/event/event.hpp"
#include "protocols/clock/clock.hpp"
#include "protocols/compositor/window.hpp"
#define EXTERNAL_INCLUDED
#include <gfx/platform/app.hpp>
#include <gfx/platform/window.hpp>
#include <string.h>

#include "gfx/backend.hpp"
#include "gfx/canvas/raster/rasterCanvas.hpp"
#include "gfx/color.hpp"

namespace wgfx
{
struct WingosWindowImpl : public wgfx::PlatformWindow
{
    RasterCanvas *raster_canvas;
    void *back_buffer;
    prot::WindowGetAttributeSize size;
    prot::WindowConnection *g_window;
    prot::HIConnection *g_inputs;
    prot::ClockConnection *g_clock;
    void *raster_buffer;
    size_t raster_width;
    size_t raster_height;

    virtual void destroy() override
    {

        switch (backend_kind)
        {
        case wgfx::BACKEND_KIND_RASTER:
        {
            delete raster_canvas;
            break;
        }
        default:
        {
            fmt::err$("Invalid backend kind used for destroy");
            break;
        }
        }
        g_window->raw_client().disconnect();
        g_inputs->raw_client().disconnect();

        attached = false;
    }

    virtual Canvas *create_frame() override
    {

        switch (backend_kind)
        {
        case wgfx::BACKEND_KIND_RASTER:
        {

            raster_canvas->bpp = 32;
            raster_canvas->width = width();
            raster_canvas->height = height();
            raster_canvas->buffer = (Rgba8 *)raster_buffer;
            raster_canvas->size.start = {0, 0};
            raster_canvas->size.end.x = raster_canvas->width;
            raster_canvas->size.end.y = raster_canvas->height;
            return (Canvas *)raster_canvas;
        }
        default:
        {
            fmt::warn$("Canvas can't create frame for backend kind: {}", (int)backend_kind);
        }
        }
        return nullptr;
    }

    virtual float dpi() override
    {
        return 1.f;
    }

    virtual void end_frame(Canvas *frame) override
    {
        frame->flush();

        switch (backend_kind)
        {
        case wgfx::BACKEND_KIND_RASTER:
        {
            RasterCanvas *cnvas = (RasterCanvas *)frame;

            memcpy(back_buffer, cnvas->buffer, cnvas->width * cnvas->height * sizeof(uint32_t));
            g_window->swap_buffers();

            // print fps
            static uint64_t oldtime = 0;
            static uint64_t newtime = 0;
            static uint64_t fps = 0;
            newtime = g_clock->get_system_time().unwrap().milliseconds;
            fps = newtime - oldtime;
            oldtime = newtime;
            (void)fps;
            //     printf("%f\n", (1000.f * 1000.f) / (float)fps);
            break;
        }
        default:
        {
            fmt::warn$("Canvas can't end frame for backend kind: {}", (int)backend_kind);
        }
        }
    }

    virtual size_t width() override
    {
        return raster_width;
    }

    virtual size_t height() override
    {
        return raster_height;
    }

    virtual core::Result<void> attach() override
    {

        switch (backend_kind)
        {
        case wgfx::BACKEND_KIND_RASTER:
        {

            auto nsize = g_window->get_attribute_size().copied();

            raster_width = nsize.width;
            raster_height = nsize.height;
            raster_buffer = new uint32_t[raster_height * raster_width];

            back_buffer = g_window->get_framebuffer().copied().ptr();
            raster_canvas = new RasterCanvas();
            break;
        }
        default:
        {
            fmt::warn$("Backend currently not supported: {}", (int)backend_kind);
            return "Backend not supported";
        }
        }
        return {};
    }

    wgfx::UEvent query_event() override
    {

        return {};
        /*
        SDL_Event ev = {};

        while (SDL_PollEvent(&ev))
        {
            if (ev.type == SDL_EVENT_QUIT)
            {
                exit(0);
            }

            else if (ev.type == SDL_EVENT_MOUSE_MOTION)
            {
                wgfx::UEvent mouse = {};

                mouse.mouse.offx = ev.motion.xrel * this->dpi();
                mouse.mouse.offy = ev.motion.yrel * this->dpi();
                mouse.at.x = ev.motion.x * this->dpi();
                mouse.at.y = ev.motion.y * this->dpi();

                mouse.kind = wgfx::UEvent::Kind::MOUSE_MOVE;

                return mouse;
            }
            else if (ev.type == SDL_EVENT_MOUSE_BUTTON_DOWN || ev.type == SDL_EVENT_MOUSE_BUTTON_UP)
            {
                wgfx::UEvent mouse = {};

                mouse.mouse.left = ev.button.button == SDL_BUTTON_LEFT;
                mouse.mouse.right = ev.button.button == SDL_BUTTON_RIGHT;
                mouse.mouse.middle = ev.button.button == SDL_BUTTON_MIDDLE;

                mouse.at.x = ev.button.x * this->dpi();
                mouse.at.y = ev.motion.y * this->dpi();

                if(ev.type == SDL_EVENT_MOUSE_BUTTON_DOWN)
                {
                    mouse.kind = wgfx::UEvent::Kind::MOUSE_CLICK;
                }
                else
                {
                    mouse.kind = wgfx::UEvent::Kind::MOUSE_RELEASE;
                }
                return mouse;

            }
            }*/

        return {};
    }
};

core::Result<core::SharedPtr<wgfx::PlatformWindow>>

wgfx::PlatformWindow::create_native(wgfx::BackendsKinds preferred_backend)
{
    core::SharedPtr<WingosWindowImpl> window = core::SharedPtr<WingosWindowImpl>::make();

    window->backend_kind = preferred_backend;

    /*
    SDL_PropertiesID props = SDL_CreateProperties();
    SDL_SetStringProperty(props, SDL_PROP_WINDOW_CREATE_TITLE_STRING, "WingOS SDL Window");
    SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER, 1920);
    SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER, 1080);
    SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_WAYLAND_SURFACE_ROLE_CUSTOM_BOOLEAN, true);
    //  SDL_SetWindowProperty(props, SDL_PROP_WINDOW_CREATE_FLAGS_NUMBER, (preferred_backend == BackendsKinds::BACKEND_KIND_OPENGL ? SDL_WINDOW_OPENGL : 0) | SDL_WINDOW_HIGH_PIXEL_DENSITY);
*/
    window->g_window = new prot::WindowConnection(prot::WindowConnection::create(true).copied());
    window->g_inputs = new prot::HIConnection(prot::HIConnection::connect().copied());
    window->g_clock = new prot::ClockConnection(prot::ClockConnection::connect().copied());

    return window.static_pointer_cast<wgfx::PlatformWindow>();
}

}; // namespace wgfx
