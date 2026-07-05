#pragma once

#include "gfx/canvas/canvas.hpp"
#include "gfx/geometry/rect.hpp"
#include "gfx/geometry/vec2.hpp"
#include "libcore/shared.hpp"
#include "ui/context.hpp"
#include "widget.hpp"

namespace fc
{

class Centered : public Widget
{

public:
    fc::SharedPtr<Widget> child;

    ~Centered() override = default;

    template <typename T>
    Centered(T args)
    {
        child = (args);
    }

    virtual wgfx::Vec2 preferred_size(wgfx::Vec2 constraint) const override
    {
        return constraint;
    }

    virtual wgfx::GRect layout(UiContext const &ctx, wgfx::GRect constraint) override
    {
        auto child_size = child->preferred_size(constraint.size());
        auto child_start = constraint.start + (constraint.size() - child_size) * 0.5f;
        auto child_end = child_start + child_size;
        child->relayout(ctx, wgfx::GRect::from_start_end(child_start, child_end));
        return constraint;
    }

    void render(UiContext const &ctx, wgfx::Canvas &canvas) const override
    {
        (void)ctx;
        (void)canvas;
    }

    template <typename T>
    static fc::SharedPtr<Widget> construct(T args)
    {
        return fc::SharedPtr<Centered>::make(args).template static_pointer_cast<Widget>();
    }

    fc::SharedPtr<Widget> build(UiContext const &v) override
    {
        (void)v;
        return child;
    };
};

} // namespace fc
