
#include <gfx/platform/app.hpp>


#include <SDL3/SDL.h>
#include <SDL3/SDL_surface.h>



core::Result<void> wgfx::initialize_platform()
{
    if (SDL_Init(SDL_INIT_VIDEO) == 0)
    {
        log::err$("SDL_Init Error: {}", SDL_GetError());
        return "failed to initialize SDL";
    }
    return {};
}
