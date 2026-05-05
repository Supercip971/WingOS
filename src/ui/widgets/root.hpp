#pragma once

#include "gfx/color.hpp"
#include "gfx/geometry/rect.hpp"
#include "gfx/geometry/vec2.hpp"
#include "gfx/text/font.hpp"
#include "libcore/ds/vec.hpp"
#include "libcore/shared.hpp"
#include "widget.hpp"

namespace fc
{

class _Root : public Widget
{

public:
    core::SharedPtr<Widget> elements;

    wgfx::CompositeColor bg;
    ~_Root() override = default;
    template <typename T>
    _Root(wgfx::CompositeColor bg, T args)
        : bg(bg)
    {
        elements = (args);
    }


    virtual wgfx::Vec2 preferred_size(wgfx::Vec2 constraint) const override
    {
        return constraint;
    }

     void render(UiContext const &ctx, wgfx::Canvas &canvas) const override
    {
        log::log$("render root: {} {} {} {}", (long)this->bounds().start.x, (long)this->bounds().start.y, (long)this->bounds().width(), (long)this->bounds().height());
        (void)ctx;
        (void)canvas;
        canvas.drawRect(this->bounds(), this->bg);
    }

    template <typename T>
    static core::SharedPtr<Widget> construct(wgfx::CompositeColor bg,  T args)
    {
        return core::SharedPtr<_Root>::make(bg, args).template static_pointer_cast<Widget>();
    }

     core::SharedPtr<Widget> build(UiContext const &v) override
    {
        (void)v;
        return elements;
    };
};




} // namespace fc
