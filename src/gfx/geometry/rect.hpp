

#include "gfx/geometry/vec2.hpp"
namespace wgfx
{
struct GRect
{
    Vec2 start;
    Vec2 end;

    constexpr void width(float value)  {
        end.x = start.x + value;
    }
    constexpr void height(float value) {
        end.y = start.y + value;
    }


    constexpr GRect(long _x, long _y, long _ex, long _ey) : start(_x, _y), end(_ex, _ey) {}

    constexpr GRect() {};

    static constexpr GRect from_start_end(long sx, long sy, long ex, long ey)
    {
        return GRect(sx, sy, ex - sx, ey - sy);
    }
    static constexpr GRect from_size(long sx, long sy, long width, long height)
    {
        return GRect(sx, sy, sx + width, sy + height);
    }
};
} // namespace wgfx
