#pragma once

#include "libcore/str_writer.hpp"

#include "gfx/canvas/canvas.hpp"
#include "gfx/canvas/cmd.hpp"
#include "gfx/color.hpp"
#include "gfx/event/event.hpp"
#include "gfx/geometry/rect.hpp"
#include "gfx/geometry/vec2.hpp"
#include "libcore/shared.hpp"
#include "ui/context.hpp"
#include "ui/widgets/statefull.hpp"
#include "widget.hpp"

namespace fc
{

struct DrageableContainerParams
{
    wgfx::CompositeColor _bg = wgfx::CONTAINER_FILL;
    wgfx::CompositeColor _border = wgfx::CONTAINER_BORDER;
    float _radius = 16.f;
    float _elevation = 12.0f;
    fc::SharedPtr<fc::WStr> _title;
    float _width = 0;
    float _height = 0;

    constexpr DrageableContainerParams()
    {
    }

    constexpr DrageableContainerParams(
        float initial_width,
        float initial_height

    )
    {
        _width = initial_width;
        _height = initial_height;
    };

    constexpr DrageableContainerParams bg(wgfx::CompositeColor bg) const
    {
        DrageableContainerParams p = *this;
        p._bg = bg;
        return p;
    }

    constexpr DrageableContainerParams border(wgfx::CompositeColor border) const
    {
        DrageableContainerParams p = *this;
        p._border = border;
        return p;
    }

    constexpr DrageableContainerParams radius(float radius) const
    {
        DrageableContainerParams p = *this;
        p._radius = radius;
        return p;
    }

    constexpr DrageableContainerParams elevation(float elevation) const
    {
        DrageableContainerParams p = *this;
        p._elevation = elevation;
        return p;
    }

    constexpr DrageableContainerParams title(fc::SharedPtr<fc::WStr> title) const
    {
        DrageableContainerParams p = *this;
        p._title = title;
        return p;
    }
};

struct DrageableState
{
    DrageableContainerParams _parms = {};
    bool _is_dragging = false;
    bool _is_resizing = false;
    float off_x = 100;
    float off_y = 100;
    float width = 1920 / 2;
    float height = 1080 / 2;
    float old_mouse_x = 0;
    float old_mouse_y = 0;
};

class DrageableContainer : public Statefull<DrageableState>
{

public:
    fc::SharedPtr<Widget> child;

    ~DrageableContainer() override = default;

    template <typename T>
    DrageableContainer(DrageableContainerParams parms, T args)
    {
        _parms = parms;
        this->width = parms._width;
        off_x = 100;
        off_y = 100;
        this->height = parms._height;
        child = (args);
    }

    virtual wgfx::Vec2 preferred_size(wgfx::Vec2) const override
    {
        wgfx::Vec2 c = {
            this->width,
            this->height,
        };

        return child->preferred_size(c);
    }

    wgfx::GRect layout(UiContext const &ctx, wgfx::GRect constraint) override
    {
        wgfx::GRect child_constraint = constraint.with_size(preferred_size(constraint.size()));
        child_constraint.start += wgfx::Vec2(off_x, off_y);
        child_constraint.end += wgfx::Vec2(off_x, off_y);
        this->child->relayout(ctx, child_constraint);

        return child_constraint;
    }

    bool acquireEvent(wgfx::UEvent ev) override
    {

        if (ev.kind == wgfx::UEvent::Kind::MOUSE_MOVE && this->_is_dragging)
        {
            setState([&]()
                     {
                off_x += ev.mouse.absx - old_mouse_x;
                off_y += ev.mouse.absy - old_mouse_y;
                old_mouse_x = ev.mouse.absx;
                old_mouse_y = ev.mouse.absy; });
            return true;
        }
        else if (ev.kind == wgfx::UEvent::Kind::MOUSE_MOVE && this->_is_resizing)
        {
            setState([&]()
                     {
                        width += ev.mouse.absx - old_mouse_x;
                        height += ev.mouse.absy - old_mouse_y;
                        old_mouse_x = ev.mouse.absx;
                        old_mouse_y = ev.mouse.absy; });
            return true;
        }
        else if (ev.kind == wgfx::UEvent::Kind::MOUSE_CLICK && (!this->_is_dragging) && (!this->_is_resizing))
        {

            setState([&]()
                     {
                         if (ev.mouse.left)
                         {
                             this->_is_dragging = true;
                         }
                         else if (ev.mouse.right)
                         {
                             this->_is_resizing = true;
                         }
                         old_mouse_x = ev.mouse.absx;
                         old_mouse_y = ev.mouse.absy; });

            return true;
        }
        else if (ev.kind == wgfx::UEvent::Kind::MOUSE_RELEASE && (this->_is_dragging || this->_is_resizing))
        {

            setState([&]()
                     { this->_is_dragging = false;
                       this->_is_resizing = false; });
            return true;
        }
        return false;
    }

    void render(UiContext const &ctx, wgfx::Canvas &canvas) const override
    {
        (void)ctx;

        wgfx::Painter paint = this->_parms._bg;

        paint.blur = 64.f;

        auto b = this->bounds();

        canvas.drawRect(this->bounds(), paint, this->_parms._radius);

        paint.type = wgfx::PaintType::PAINT_MODE_STROKE;
        paint.stroke.width = 2.f * 2.f;
        paint.color = this->_parms._border;

        canvas.drawRect(b, paint, this->_parms._radius);
    }

    template <typename T>
    static fc::SharedPtr<Widget> construct(DrageableContainerParams params, T args)
    {
        return fc::SharedPtr<DrageableContainerParams>::make(params, args).template static_pointer_cast<Widget>();
    }

    fc::SharedPtr<Widget> build(UiContext const &) override
    {
        return child;
    };
};

} // namespace fc
