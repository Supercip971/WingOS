#pragma once

#include "libcore/str_writer.hpp"

#include "gfx/canvas/cmd.hpp"
#include "gfx/color.hpp"
#include "gfx/event/event.hpp"
#include "gfx/geometry/rect.hpp"
#include "gfx/geometry/vec2.hpp"
#include "gfx/text/font.hpp"
#include "libcore/ds/vec.hpp"
#include "libcore/shared.hpp"
#include "ui/widgets/callback.hpp"
#include "ui/widgets/centered.hpp"
#include "ui/widgets/statefull.hpp"
#include "widget.hpp"

namespace fc
{

using callbackType = decltype([]() {});
struct ButtonParams
{
    wgfx::CompositeColor _bg = wgfx::CONTAINER_FILL;
    wgfx::CompositeColor _border = wgfx::CONTAINER_BORDER;
    wgfx::CompositeColor _shadowy = wgfx::CONTAINER_BORDER;
    float _radius = 0.1f;
    float _elevation = 12.0f;

    constexpr ButtonParams() = default;

    constexpr ButtonParams bg(wgfx::CompositeColor bg) const
    {
        ButtonParams p = *this;
        p._bg = bg;
        return p;
    }

    constexpr ButtonParams border(wgfx::CompositeColor border) const
    {
        ButtonParams p = *this;
        p._border = border;
        return p;
    }

    constexpr ButtonParams radius(float radius) const
    {
        ButtonParams p = *this;
        p._radius = radius;
        return p;
    }

    constexpr ButtonParams shadowy(wgfx::CompositeColor shadowy) const
    {
        ButtonParams p = *this;
        p._shadowy = shadowy;
        return p;
    }

    constexpr ButtonParams elevation(float elevation) const
    {
        ButtonParams p = *this;
        p._elevation = elevation;
        return p;
    }
};

struct ButtonState
{
    ButtonParams _parms;
};

class Button : public Statefull<ButtonState>
{

public:
    core::SharedPtr<Widget> child;
    float o_elevation;

    Callback _on_click = nullptr;
    ~Button() override = default;
    template <typename T>
    Button(ButtonParams parms, Callback callback, T args)
    {
        o_elevation = parms._elevation;

        _on_click = callback;
        _parms = parms;
        child = (args);
    }

    bool acquireEvent(wgfx::UEvent ev) override
    {
        /*
        if (ev.kind == wgfx::UEvent::Kind::MOUSE_MOVE)
        {
            setState([&]()
                     { _parms._elevation = 0.5 * o_elevation; });
            return true;
            }*/
        if (ev.kind == wgfx::UEvent::Kind::MOUSE_CLICK)
        {

            setState([&]()
                     { _parms._elevation = 0; });

            _on_click.call();
            return true;
        }
        else if (_parms._elevation != o_elevation)
        {

            setState([&]()
                     { _parms._elevation = o_elevation; });
            return true;
        }
        return false;
    }
    virtual wgfx::Vec2 preferred_size(wgfx::Vec2 constraint) const override
    {

        wgfx::Vec2 c2 = constraint;
        c2.y -= this->_parms._elevation;
        return child->preferred_size(c2) + wgfx::Vec2(0, this->_parms._elevation);
    }

    void render(UiContext const &ctx, wgfx::Canvas &canvas) const override
    {
        (void)ctx;

        wgfx::Painter paint = this->_parms._shadowy;

        auto b = this->bounds();
        
        canvas.drawRect(this->bounds(), paint, this->_parms._radius);

        auto bu = this->bounds();
        bu.end.y -= this->_parms._elevation;
        paint.color = this->_parms._bg;
        canvas.drawRect(bu, paint, this->_parms._radius);

        paint.type = wgfx::PaintType::PAINT_MODE_STROKE;
        paint.stroke.width = 2.f * 2.f;
        paint.color = this->_parms._border;

        canvas.drawRect(bu, paint, this->_parms._radius);
        canvas.drawRect(b, paint, this->_parms._radius);
    }

    template <typename T>
    static core::SharedPtr<Widget> construct(ButtonParams params, T args)
    {
        return core::SharedPtr<ButtonParams>::make(params, args).template static_pointer_cast<Widget>();
    }

    core::SharedPtr<Widget> build(UiContext const &v) override
    {
        (void)v;
        return $<fc::Centered>(child);
    };
};

} // namespace fc
