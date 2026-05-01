#pragma once

#include <libcore/core.hpp>
#include "gfx/color.hpp"
#include "gfx/geometry/rect.hpp"
#include "gfx/geometry/shape.hpp"

#include "gfx/text/font.hpp"
#include "gfx/text/utf-text.hpp"
#include "libcore/shared.hpp"
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
    RENDER_KIND_CONTOUR,
    RENDER_KIND_SHAPE,

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
    float radius;
};

struct ContourCommand
{
    static constexpr auto KIND = RenderCommandKind::RENDER_KIND_CONTOUR;

    Painter paint = {};
    core::SharedPtr<wgfx::Contour> contour = {};

    Vec2 pos = {};
};

struct TextCommand
{

    static constexpr auto KIND = RenderCommandKind::RENDER_KIND_TEXT;

    Painter paint;
    Utf8Str str;
    core::SharedPtr<Font> font;
    Vec2 pos;

};


struct RenderCommand
{
public:
    RenderCommandKind kind;
    union
    {

        TextCommand text;
        FillCommand fill;
        ContourCommand contour;
        RectCommand rect;

    };

    // Default constructor
    RenderCommand() : kind(RenderCommandKind::RENDER_KIND_FILL) {}

    // Copy constructor
    RenderCommand(const RenderCommand& other) : kind(other.kind)
    {
        switch (kind)
        {
            case RenderCommandKind::RENDER_KIND_FILL:
                fill = other.fill;
                break;
            case RenderCommandKind::RENDER_KIND_TEXT:
                new (&text) TextCommand(other.text);
                break;
            case RenderCommandKind::RENDER_KIND_RECT:
                rect = other.rect;
                break;
            case RenderCommandKind::RENDER_KIND_USE_COLOR:
                break;
            case RenderCommandKind::RENDER_KIND_CONTOUR:
                new (&contour) ContourCommand(other.contour);
                break;
            default:
                break;
        }
    }

    // Copy assignment operator
    RenderCommand& operator=(const RenderCommand& other)
    {
        if (this != &other)
        {
            this->~RenderCommand();
            kind = other.kind;
            switch (kind)
            {
                case RenderCommandKind::RENDER_KIND_FILL:
                    fill = other.fill;
                    break;
                case RenderCommandKind::RENDER_KIND_TEXT:
                    new (&text) TextCommand(other.text);
                    break;
                case RenderCommandKind::RENDER_KIND_RECT:
                    rect = other.rect;
                    break;
                case RenderCommandKind::RENDER_KIND_USE_COLOR:
                    break;
                case RenderCommandKind::RENDER_KIND_CONTOUR:
                    new (&contour) ContourCommand(other.contour);
                    break;
                default:
                    break;
            }
        }
        return *this;
    }

    // Destructor
    ~RenderCommand()
    {
        switch (kind)
        {
            case RenderCommandKind::RENDER_KIND_TEXT:
                text.~TextCommand();
                break;
            case RenderCommandKind::RENDER_KIND_CONTOUR:
                contour.~ContourCommand();
                break;
            default:
                break;
        }
    }

    template<typename T>
    static RenderCommand from(const T& r)
    {
        RenderCommand cmd = {};
        cmd.kind = T::KIND;
        if constexpr (T::KIND == RenderCommandKind::RENDER_KIND_TEXT)
        {
            new (&cmd.text) TextCommand(r);
        }
        else if constexpr (T::KIND == RenderCommandKind::RENDER_KIND_RECT)
        {
            new (&cmd.rect) RectCommand(r);
        }
        else if constexpr (T::KIND == RenderCommandKind::RENDER_KIND_FILL)
        {
            new (&cmd.fill) FillCommand(r);
        }
        else if constexpr (T::KIND == RenderCommandKind::RENDER_KIND_CONTOUR)
        {
            new (&cmd.contour) ContourCommand(r);
        }
        return cmd;
    }
};

} // namespace wgfx
