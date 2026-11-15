#pragma once

#include "gfx/geometry/vec2.hpp"
#include "hw/hi/mouse.hpp"

namespace wgfx
{

class UEvent
{

public:
    enum class Kind
    {
        NONE,
        MOUSE_MOVE,
        MOUSE_CLICK,
        MOUSE_RELEASE,
    };

    Kind kind{Kind::NONE};
    wgfx::Vec2 at;

    union
    {
        hw::MouseEvent mouse;
    };

    UEvent() : at(0, 0), mouse({.absx = 0, .absy = 0, .offx = 0, .offy = 0, .scroll = 0, .left = false, .right = false, .middle = false}) {};
};

} // namespace wgfx
