#pragma once

#include "gfx/color.hpp"
namespace wgfx
{

enum PaintType
{
    PAINT_MODE_FILLED,
    PAINT_MODE_STROKE
};
struct Painter
{
    // Complete Color, is raw encoded color (with HDR and everything), then converted to rawColor (rgba 255bit)
    CompositeColor color;
    PaintType type;

    constexpr Painter() {}
    constexpr Painter(CompositeColor comp_color)
    {
        color = comp_color;
        type = PAINT_MODE_FILLED;
    };
    constexpr static Painter filled(CompositeColor col) { return Painter(col); };
};
enum class RenderCommandKind
{
    RENDER_KIND_FILL,
    RENDER_KIND_USE_COLOR,
    RENDER_KIND_TEXT,
};

struct FillCommand
{
    Painter paint;
};
struct RenderCommand
{

    RenderCommandKind kind;
    union
    {
        FillCommand fill;
    };
};
} // namespace wgfx
