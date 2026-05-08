#pragma once

#include "gfx/canvas/cmd.hpp"
#include "gfx/color.hpp"
#include "gfx/geometry/rect.hpp"
#include "gfx/geometry/vec2.hpp"
#include "gfx/text/font.hpp"
#include "libcore/ds/vec.hpp"
#include "libcore/shared.hpp"
#include "widget.hpp"

namespace fc
{

struct LayoutSize
{

    float min_w = -1;
    float max_w = -1;
    float min_h = -1;
    float max_h = -1;

    LayoutSize() = default;

    constexpr LayoutSize(float min_w, float max_w, float min_h, float max_h)
        : min_w(min_w), max_w(max_w), min_h(min_h), max_h(max_h)
    {
    }

    constexpr LayoutSize(float w, float h)
        : min_w(w), max_w(w), min_h(h), max_h(h)
    {
    }

    constexpr LayoutSize min_width(float w) const
    {
        LayoutSize s = *this;
        s.min_w = w;
        return s;
    }

    constexpr LayoutSize max_width(float w) const
    {
        LayoutSize s = *this;
        s.max_w = w;
        return s;
    }

    constexpr LayoutSize min_height(float h) const
    {
        LayoutSize s = *this;
        s.min_h = h;
        return s;
    }

    constexpr LayoutSize max_height(float h) const
    {
        LayoutSize s = *this;
        s.max_h = h;
        return s;
    }
};

class Sized : public Widget
{

public:
    core::SharedPtr<Widget> child;

    LayoutSize _parms;

    ~Sized() override = default;
    template <typename T>
    Sized(LayoutSize parms, T args)
        : _parms(parms)
    {
        child = (args);
    }

    virtual wgfx::Vec2 preferred_size(wgfx::Vec2 constraint) const override
    {

        constraint.x = _parms.max_w >= 0 ? core::min(constraint.x, _parms.max_w) : constraint.x;
        constraint.y = _parms.max_h >= 0 ? core::min(constraint.y, _parms.max_h) : constraint.y;

        constraint.x = _parms.min_w >= 0 ? core::max(constraint.x, _parms.min_w) : constraint.x;
        constraint.y = _parms.min_h >= 0 ? core::max(constraint.y, _parms.min_h) : constraint.y;

        return child->preferred_size(constraint);
    }
    virtual wgfx::GRect layout(UiContext const &ctx, wgfx::GRect constraint) override
    {
        // fmt::log$("== LAYOUT == ");
        // dump();
        fmt::log$("Constraint: ({}x{} - {}x{})", (long)constraint.start.x, (long)constraint.start.y, (long)constraint.end.x, (long)constraint.end.y);

        (void)ctx;


        wgfx::GRect child_constraint = constraint.with_size(preferred_size(constraint.size()));
        child->relayout(ctx, child_constraint);

        return constraint.with_size(preferred_size(constraint.size()));
    }
    void render(UiContext const &ctx, wgfx::Canvas &canvas) const override
    {
        (void)ctx;
        (void)canvas;
    }

    template <typename T>
    static core::SharedPtr<Widget> construct(LayoutSize params, T args)
    {
        return core::SharedPtr<Sized>::make(params, args).template static_pointer_cast<Widget>();
    }

    core::SharedPtr<Widget> build(UiContext const &v) override
    {
        (void)v;
        return child;
    };
};

} // namespace fc
