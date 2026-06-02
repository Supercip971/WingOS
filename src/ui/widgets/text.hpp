#pragma once

#include "libcore/str_writer.hpp"

#include "gfx/color.hpp"
#include "gfx/geometry/rect.hpp"
#include "gfx/geometry/vec2.hpp"
#include "gfx/text/font.hpp"
#include "libcore/shared.hpp"
#include "widget.hpp"

namespace fc
{
class TextWidget : public Widget
{

    core::WStr val;

    core::SharedPtr<wgfx::Font> font;

public:
    ~TextWidget() override = default;
    TextWidget(core::Str from, core::SharedPtr<wgfx::Font> _font) : val(core::WStr::copy(from)), font(_font) {}

    TextWidget(core::WStr &&from, core::SharedPtr<wgfx::Font> _font) : val(core::move(from)), font(_font) {}

    wgfx::Vec2 preferred_size(wgfx::Vec2 constraint) const override
    {


        (void)constraint;
        auto v = font->get_render_rect(val.view());

        //v.y = core::max(v.y, constraint.y);

        return v;
    }

    core::Str info() const override { return val.view(); }
    void render(UiContext const &ctx, wgfx::Canvas &canvas) const override
    {
        fmt::log$("renderign text at: {}-{} {}", (long)bounds().start.x, (long)bounds().start.y, val.view());
        (void)ctx;
        canvas.drawText(bounds().start + wgfx::Vec2(0.f, 96.f), val.view(), font, wgfx::CONTAINER_TEXT);
    }
};
} // namespace fc
