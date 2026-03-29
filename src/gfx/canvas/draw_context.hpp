#pragma once


#include "gfx/canvas/cmd.hpp"
#include "libcore/optional.hpp"

namespace wgfx {

    class DrawContext
    {
        core::Optional<wgfx::GRect> scissor;

    };
}
