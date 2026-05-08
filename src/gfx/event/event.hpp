#pragma once



#include "gfx/geometry/vec2.hpp"
#include "hw/hi/mouse.hpp"
#include "libcore/str.hpp"
namespace wgfx
{

    class UEvent
    {

        public:

        core::Str kind;
    };


    class UEventMouse : public UEvent
    {
        public:

        wgfx::Vec2 at;

        hw::MouseEvent mev;

        static core::Str kind()
        {
            return "mouse";
        }
    };
}
