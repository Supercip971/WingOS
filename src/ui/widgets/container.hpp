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

struct ContainerParms
{
    wgfx::CompositeColor _bg = wgfx::CONTAINER_FILL;
    wgfx::CompositeColor _border = wgfx::CONTAINER_BORDER;
    float _border_size = 0.2f;

    float _elevation = 0.f;
    float _radius = 0.15f;

    constexpr ContainerParms() = default;

    constexpr ContainerParms bg(wgfx::CompositeColor bg) const
    {
        ContainerParms p = *this;
        p._bg = bg;
        return p;
    }

    constexpr ContainerParms border(wgfx::CompositeColor border, float size) const
    {
        ContainerParms p = *this;
        p._border = border;
        p._border_size = size;
        return p;
    }

    constexpr ContainerParms elevation(float elevation) const
    {
        ContainerParms p = *this;
        p._elevation = elevation;
        return p;
    }

    constexpr ContainerParms radius(float radius) const
    {
        ContainerParms p = *this;
        p._radius = radius;
        return p;
    }
};

class Container : public Widget
{

public:
    core::SharedPtr<Widget> child;

    ContainerParms _parms;

    ~Container() override = default;
    template <typename T>
    Container(ContainerParms parms, T args)
        : _parms(parms)
    {
        child = (args);
    }

    virtual wgfx::Vec2 preferred_size(wgfx::Vec2 constraint) const override
    {
        return constraint;
    }

    void render(UiContext const &ctx, wgfx::Canvas &canvas) const override
    {
        (void)ctx;

        wgfx::Painter paint = this->_parms._bg;

        auto b = this->bounds();

        canvas.drawRect(this->bounds(), paint, this->_parms._radius);
        if (this->_parms._border_size > 0.f)
        {

            paint.type = wgfx::PaintType::PAINT_MODE_STROKE;
            paint.stroke.width = this->_parms._border_size * 2.f;
            paint.color = this->_parms._border;

            canvas.drawRect(b, paint, this->_parms._radius);
        }
    }

    template <typename T>
    static core::SharedPtr<Widget> construct(ContainerParms params, T args)
    {
        return core::SharedPtr<Container>::make(params, args).template static_pointer_cast<Widget>();
    }

    core::SharedPtr<Widget> build(UiContext const &v) override
    {
        (void)v;
        return child;
    };
};

} // namespace fc
