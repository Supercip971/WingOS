#pragma once

#include "gfx/canvas/canvas.hpp"
#include "gfx/color.hpp"
#include "gfx/geometry/rect.hpp"
#include "gfx/geometry/vec2.hpp"
#include "libcore/fmt/log.hpp"
#include "libcore/shared.hpp"
#include "ui/context.hpp"
#include "widget.hpp"

namespace fc
{

class _Root : public Widget
{

public:
    fc::SharedPtr<Widget> elements;

    wgfx::CompositeColor bg;
    ~_Root() override = default;

    template <typename T>
    _Root(wgfx::CompositeColor _bg, T args)
        : bg(_bg)
    {
        elements = (args);
    }

    virtual wgfx::Vec2 preferred_size(wgfx::Vec2 constraint) const override
    {
        return constraint;
    }

    void render(UiContext const &ctx, wgfx::Canvas &canvas) const override
    {
        fmt::log$("render root: {} {} {} {}", (long)this->bounds().start.x, (long)this->bounds().start.y, (long)this->bounds().width(), (long)this->bounds().height());
        (void)ctx;
        (void)canvas;
        canvas.drawRect(this->bounds(), this->bg);
    }

    template <typename T>
    static fc::SharedPtr<Widget> construct(wgfx::CompositeColor bg, T args)
    {
        return fc::SharedPtr<_Root>::make(bg, args).template static_pointer_cast<Widget>();
    }

    fc::SharedPtr<Widget> build(UiContext const &v) override
    {
        (void)v;
        return elements;
    };
};

} // namespace fc
