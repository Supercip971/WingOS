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

class VFlex : public Widget
{

public:
    core::Vec<core::SharedPtr<Widget>> elements;

    ~VFlex() override = default;
    template <typename... T>
    VFlex(T... args)
    {
        elements = core::Vec<core::SharedPtr<Widget>>(args...);
    }

    wgfx::Vec2 preferred_size(wgfx::Vec2 constraint) const override
    {
        wgfx::Vec2 current_constraint = {};
        for (auto &child : childs)
        {
            auto c = child->preferred_size(constraint);
            current_constraint.y += c.y;
            current_constraint.x = core::max(current_constraint.x, c.x);
            constraint.y -= c.y;
            constraint.x = core::max(constraint.x, c.x);
        }
        return current_constraint;
    }

    wgfx::GRect layout(UiContext const &ctx, wgfx::GRect constraint) override
    {
        wgfx::GRect current_constraint = constraint;
        for (auto &child : childs)
        {
            auto child_constraint = child->preferred_size(current_constraint.size());
            child->relayout(ctx, current_constraint);
            current_constraint.start.y += child_constraint.y;
        }
        return wgfx::GRect::from_start_end(
            constraint.start.x, constraint.start.y, constraint.end.x, current_constraint.start.y);
    }

    void render(UiContext const &ctx, wgfx::Canvas &canvas) const override
    {
        (void)ctx;
        (void)canvas;
        //  for (auto &child : childs)
        //  {
        //      child->render(ctx, canvas);
        //  }
    }

    core::Vec<core::SharedPtr<Widget>> build_childs(UiContext const &v) override
    {
        (void)v;
        return elements;
    };
};
} // namespace fc
