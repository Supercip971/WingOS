#include <SDL3/SDL.h>
#include <gfx/platform/app.hpp>
#include <gfx/platform/window.hpp>

#include <SDL3/SDL_oldnames.h>
#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_surface.h>
#include <SDL3/SDL_video.h>

#include "gfx/backend.hpp"
#include "gfx/canvas/opengl/openglCanvas.hpp"
#include "gfx/canvas/raster/rasterCanvas.hpp"
#include "gfx/color.hpp"

namespace wgfx
{
struct SDLWindowImpl : public wgfx::PlatformWindow
{
    RasterCanvas* raster_canvas;
    OpenglCanvas* opengl_canvas;

    SDL_Window *window;

    SDL_GLContext ctx;

    SDL_Renderer *renderer;

    // raster ctx

    SDL_Texture *raster_texture;

    void *raster_buffer;
    size_t raster_width;
    size_t raster_height;

    virtual void destroy() override
    {

        switch (backend_kind)
        {
        case wgfx::BACKEND_KIND_RASTER:
        {
            SDL_DestroyTexture(raster_texture);
            SDL_DestroyRenderer(renderer);
            break;
        }
        case wgfx::BACKEND_KIND_OPENGL:
        {
            SDL_GL_DestroyContext(ctx);
            break;
        }

        default:
        {
            log::err$("Invalid backend kind used for destroy");
            break;
        }
        }
        SDL_DestroyWindow(window);
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
            raster_canvas->buffer = (Rgba8*)raster_buffer;
            return raster_canvas;
        }
        case wgfx::BACKEND_KIND_OPENGL:
        {

            opengl_canvas->width = width();
            opengl_canvas->height = height();
            return opengl_canvas;
        }
        default:
        {
            log::warn$("Canvas can't create frame for backend kind: {}", (int)backend_kind);
        }
        }
        return nullptr;
    }

    virtual void end_frame(Canvas *frame) override
    {
        frame->flush();
        switch (backend_kind)
        {
        case wgfx::BACKEND_KIND_OPENGL:
        {
            SDL_GL_SwapWindow(window);
            break;
        }
        case wgfx::BACKEND_KIND_RASTER:
        {
            RasterCanvas *cnvas = (RasterCanvas *)frame;
            SDL_UpdateTexture(raster_texture, NULL, cnvas->buffer, cnvas->width * sizeof(uint32_t));
            SDL_RenderClear(renderer);

            SDL_RenderTexture(renderer, raster_texture, NULL, NULL);
            SDL_RenderPresent(renderer);
            // print fps
            static uint32_t oldtime = 0;
            static uint32_t newtime = 0;
            static uint32_t fps = 0;
            newtime = SDL_GetTicks();
            fps = newtime - oldtime;
            oldtime = newtime;
            printf("%f\n", 1000.f / (float)fps);
            break;
        }
        default:
        {
            log::warn$("Canvas can't end frame for backend kind: {}", (int)backend_kind);
        }
        }
    }

    virtual size_t width() override
    {
        return 1920;
    }

    virtual size_t height() override
    {
        return 1080;
    }

    virtual core::Result<void> attach() override
    {

        switch (backend_kind)
        {
        case wgfx::BACKEND_KIND_OPENGL:
        {

            // Create window with graphics context
            SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
            SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
            SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
            SDL_GLContext gl_context = SDL_GL_CreateContext(window);
            if (gl_context == nullptr)
            {
                log::err$("SDL_GL_CreateContext(): {}", SDL_GetError());
                return SDL_GetError();
            }

            SDL_GL_MakeCurrent(window, gl_context);
            SDL_GL_SetSwapInterval(1); // Enable vsync
            SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);


            opengl_canvas = new OpenglCanvas();
            break;
        }
        case wgfx::BACKEND_KIND_RASTER:
        {
            renderer = SDL_CreateRenderer(window, NULL);
            SDL_RenderClear(renderer);
            raster_width = 1920;
            raster_height = 1080;
            raster_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, raster_width, raster_height);
            raster_buffer = new uint32_t[raster_height * raster_width];

            raster_canvas = new RasterCanvas();
            break;
        }
        default:
        {
            log::warn$("Backend currently not supported: {}", (int)backend_kind);
            return "Backend not supported";
        }
        }
        return {};
    }
};

core::Result<core::SharedPtr<wgfx::PlatformWindow>>

wgfx::PlatformWindow::create_native(wgfx::BackendsKinds preferred_backend)
{
    core::SharedPtr<SDLWindowImpl> window = core::SharedPtr<SDLWindowImpl>::make();

    window->backend_kind = preferred_backend;

    SDL_WindowFlags flags = (preferred_backend == BackendsKinds::BACKEND_KIND_OPENGL ? SDL_WINDOW_OPENGL : 0);
    window->window = SDL_CreateWindow("WingOS SDL Window", 1920, 1080, flags);

    return window.static_pointer_cast<wgfx::PlatformWindow>();
}

}; // namespace wgfx
