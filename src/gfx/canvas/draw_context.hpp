#pragma once

#include "gfx/geometry/rect.hpp"
#include "libcore/optional.hpp"

namespace wgfx
{

class DrawContext
{
    core::Optional<wgfx::GRect> scissor;
};
} // namespace wgfx
