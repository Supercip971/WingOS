
#include <gfx/platform/app.hpp>

#include <SDL3/SDL_error.h>
#include <SDL3/SDL_init.h>

#include "libcore/fmt/log.hpp"
#include "libcore/result.hpp"

core::Result<void> wgfx::initialize_platform()
{
    if (SDL_Init(SDL_INIT_VIDEO) == 0)
    {
        fmt::err$("SDL_Init Error: {}", SDL_GetError());
        return "failed to initialize SDL";
    }
    return {};
}
