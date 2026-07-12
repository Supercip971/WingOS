#pragma once

#include "libcore/str_writer.hpp"

#include "gfx/canvas/canvas.hpp"
#include "gfx/color.hpp"
#include "gfx/geometry/rect.hpp"
#include "gfx/geometry/vec2.hpp"
#include "gfx/text/font.hpp"
#include "libcore/fmt/log.hpp"
#include "libcore/shared.hpp"
#include "libcore/str.hpp"
#include "libcore/type-utils.hpp"
#include "ui/context.hpp"
#include "widget.hpp"

namespace fc
{
class TextWidget : public Widget
{

    fc::WStr val;

    fc::SharedPtr<wgfx::Font> font;

public:
    ~TextWidget() override = default;

    TextWidget(fc::Str from, fc::SharedPtr<wgfx::Font> _font) : val(fc::WStr::copy(from)), font(_font) {}

    TextWidget(fc::WStr &&from, fc::SharedPtr<wgfx::Font> _font) : val(std::move(from)), font(_font) {}

    wgfx::Vec2 preferred_size(wgfx::Vec2 constraint) const override
    {

        (void)constraint;
        auto v = font->get_render_rect(val.view());

        // v.y = fc::max(v.y, constraint.y);

        return v;
    }

    fc::Str info() const override { return val.view(); }

    void render(UiContext const &ctx, wgfx::Canvas &canvas) const override
    {
        fmt::log$("renderign text at: {}-{} {}", (long)bounds().start.x, (long)bounds().start.y, val.view());
        (void)ctx;
        canvas.drawText(bounds().start + wgfx::Vec2(0.f, font->ascent + font->descent + font->line_gap), val.view(), font, wgfx::CONTAINER_TEXT);
    }
};
} // namespace fc
