#pragma once

#include "gfx/geometry/vec2.hpp"
#include "libcore/logic.hpp"
namespace wgfx
{
struct GRect
{
    Vec2 start;
    Vec2 end;

    constexpr bool contains(const Vec2 &p) const
    {
        return p.x >= start.x && p.x <= end.x && p.y >= start.y && p.y <= end.y;
    }

    constexpr GRect resized_to_contain(Vec2 pos) const
    {
        GRect new_rect = *this;
        if (pos.x < start.x)
            new_rect.start.x = pos.x;
        if (pos.y < start.y)
            new_rect.start.y = pos.y;
        if (pos.x > end.x)
            new_rect.end.x = pos.x;
        if (pos.y > end.y)
            new_rect.end.y = pos.y;

        return new_rect;
    }
    constexpr void width(float value)
    {
        end.x = start.x + value;
    }
    constexpr void height(float value)
    {
        end.y = start.y + value;
    }

    constexpr float width() const
    {
        return end.x - start.x;
    }
    constexpr float height() const
    {
        return end.y - start.y;
    }

    Vec2 size() const
    {
        return end - start;
    }

    GRect with_size(const Vec2 &size) const
    {
        return GRect(start, start + size);
    }

    GRect intersect(const GRect &other) const
    {
        return GRect(
            core::max(start.x, other.start.x),
            core::max(start.y, other.start.y),
            core::min(end.x, other.end.x),
            core::min(end.y, other.end.y));
    }

    GRect merge(const GRect &other) const
    {
        return GRect(
            core::min(start.x, other.start.x),
            core::min(start.y, other.start.y),
            core::max(end.x, other.end.x),
            core::max(end.y, other.end.y));
    }
    Vec2 contained(const Vec2 &p) const
    {
        return Vec2(
            core::max(start.x, core::min(p.x, end.x)),
            core::max(start.y, core::min(p.y, end.y)));
    }

    constexpr GRect(float _x, float _y, float _ex, float _ey) : start(_x, _y), end(_ex, _ey) {}

    constexpr GRect(const Vec2 &start, const Vec2 &end) : start(start), end(end) {}
    constexpr GRect() {};

    static constexpr GRect from_start_end(Vec2 start, Vec2 end)
    {
        return GRect(start, end);
    }
    static constexpr GRect from_start_end(float sx, float sy, float ex, float ey)
    {
        return GRect(sx, sy, ex , ey);
    }
    static constexpr GRect from_size(float sx, float sy, float width, float height)
    {
        return GRect(sx, sy, sx + width, sy + height);
    }

    bool operator==(const GRect &other) const
    {
        return start == other.start && end == other.end;
    }
    bool operator!=(const GRect &other) const
    {
        return start != other.start || end != other.end;
    }

    GRect operator+(const Vec2 &offset) const
    {
        return GRect(start + offset, end + offset);
    }
    GRect operator-(const Vec2 &offset) const
    {
        return GRect(start - offset, end - offset);
    }

    GRect operator+=(const Vec2 &offset)
    {
        start += offset;
        end += offset;
        return *this;
    }
    GRect operator-=(const Vec2 &offset)
    {
        start -= offset;
        end -= offset;
        return *this;
    }
};
} // namespace wgfx
