#pragma once



#include "gfx/geometry/vec2.hpp"
#include "hw/hi/mouse.hpp"
#include "libcore/str.hpp"
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

        Kind kind;
        wgfx::Vec2 at;
        union  {
            hw::MouseEvent mouse;
        };

        UEvent() : kind(Kind::NONE){};

    };

}
