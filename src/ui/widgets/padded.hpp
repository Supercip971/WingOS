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

struct Padded
{

    float _pleft = 0;
    float _pright = 0;
    float _pdown = 0;
    float _ptop = 0;

    Padded() = default;

    constexpr Padded(float pleft, float pright, float pdown, float ptop)
        : _pleft(pleft), _pright(pright), _pdown(pdown), _ptop(ptop)
    {
    }

    constexpr Padded left(float w) const
    {

        Padded s = *this;
        s._pleft = w;
        return s;
    }

    constexpr Padded right(float w) const
    {
        Padded s = *this;
        s._pright = w;
        return s;
    }

    constexpr Padded down(float h) const
    {
        Padded s = *this;
        s._pdown = h;
        return s;
    }

    constexpr Padded top(float h) const
    {
        Padded s = *this;
        s._ptop = h;
        return s;
    }

    constexpr Padded horizontal(float w) const
    {
        Padded s = *this;
        s._pleft = w;
        s._pright = w;
        return s;
    }

    constexpr Padded vertical(float h) const
    {
        Padded s = *this;
        s._pdown = h;
        s._ptop = h;
        return s;
    }
};

class LPadded : public Widget
{

public:
    core::SharedPtr<Widget> child;

    Padded _parms;

    ~LPadded() override = default;

    template <typename T>
    LPadded(Padded parms, T args)
        : _parms(parms)
    {
        child = (args);
    }

    virtual wgfx::Vec2 preferred_size(wgfx::Vec2 constraint) const override
    {

        /*         constraint.x =  core::max(constraint.x - _parms._pleft, 0);
                constraint.x =  core::max(constraint.x - _parms._pright, 0);
                constraint.y =  core::max(constraint.y - _parms._ptop,0);
                constraint.y =  core::max(constraint.y - _parms._pdown,0);
                auto c =  child->preferred_size(constraint);

                c.x += _parms._pleft + _parms._pright;
                c.y += _parms._ptop + _parms._pdown;*/

        auto c = child->preferred_size(constraint);
        c.x += _parms._pleft + _parms._pright;
        c.y += _parms._ptop + _parms._pdown;
        return c;
    }

    virtual wgfx::GRect layout(UiContext const &ctx, wgfx::GRect constraint) override
    {
        // fmt::log$("== LAYOUT == ");
        // dump();
        fmt::log$("Constraint: ({}x{} - {}x{})", (long)constraint.start.x, (long)constraint.start.y, (long)constraint.end.x, (long)constraint.end.y);

        (void)ctx;

        wgfx::GRect inner_constraint = constraint;
        inner_constraint.start.x += _parms._pleft;
        inner_constraint.start.y += _parms._ptop;
        inner_constraint.end.x -= _parms._pright;
        inner_constraint.end.y -= _parms._pdown;
        auto csize = child->preferred_size(inner_constraint.size());

        child->relayout(ctx, inner_constraint.with_size(csize));

        return constraint;
    }

    void render(UiContext const &ctx, wgfx::Canvas &canvas) const override
    {
        (void)ctx;
        (void)canvas;
    }

    template <typename T>
    static core::SharedPtr<Widget> construct(Padded params, T args)
    {
        return core::SharedPtr<Padded>::make(params, args).template static_pointer_cast<Widget>();
    }

    core::SharedPtr<Widget> build(UiContext const &v) override
    {
        (void)v;
        return child;
    };
};

} // namespace fc
