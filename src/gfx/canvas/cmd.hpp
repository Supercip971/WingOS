#pragma once

#include "gfx/color.hpp"
#include "gfx/geometry/rect.hpp"
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
    RENDER_KIND_RECT,
    RENDER_KIND_USE_COLOR,
    RENDER_KIND_TEXT,

};

struct FillCommand
{
    static constexpr auto KIND = RenderCommandKind::RENDER_KIND_FILL;

    Painter paint;
};

struct RectCommand
{
    static constexpr auto KIND = RenderCommandKind::RENDER_KIND_RECT;

    Painter paint;
    GRect rect;
};


struct RenderCommand
{

    RenderCommandKind kind;
    union
    {
        FillCommand fill;
        RectCommand rect;

        uint8_t raw[];
    };
    template<typename T>
    static constexpr RenderCommand from(const T& r)
    {
        RenderCommand cmd = {};
        cmd.kind = T::KIND;
        *(T*)cmd.raw = r;
        return cmd;
    }
};


} // namespace wgfx
