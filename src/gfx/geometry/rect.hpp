

#include "gfx/geometry/vec2.hpp"
#include "libcore/logic.hpp"
namespace wgfx
{
struct GRect
{
    Vec2 start;
    Vec2 end;


    constexpr bool contains(const Vec2 & p) const {
        return p.x >= start.x && p.x <= end.x && p.y >= start.y && p.y <= end.y;
    }

    constexpr GRect resized_to_contain(Vec2 pos) const {
        GRect new_rect = *this;
        if (pos.x < start.x) new_rect.start.x = pos.x;
        if (pos.y < start.y) new_rect.start.y = pos.y;
        if (pos.x > end.x) new_rect.end.x = pos.x;
        if (pos.y > end.y) new_rect.end.y = pos.y;

        return new_rect;
    }
    constexpr void width(float value)  {
        end.x = start.x + value;
    }
    constexpr void height(float value) {
        end.y = start.y + value;
    }

    constexpr float width() const {
        return end.x - start.x;
    }
    constexpr float height() const {
        return end.y - start.y;
    }

    Vec2 contained(const Vec2 & p) const {
        return Vec2(
            core::max(start.x, core::min(p.x, end.x)),
            core::max(start.y, core::min(p.y, end.y))
        );
    }

    constexpr GRect(float _x, float _y, float _ex, float _ey) : start(_x, _y), end(_ex, _ey) {}

    constexpr GRect() {};

    static constexpr GRect from_start_end(float sx, float sy, float ex, float ey)
    {
        return GRect(sx, sy, ex - sx, ey - sy);
    }
    static constexpr GRect from_size(float sx, float sy, float width, float height)
    {
        return GRect(sx, sy, sx + width, sy + height);
    }
};
} // namespace wgfx
