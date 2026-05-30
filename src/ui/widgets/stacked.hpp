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

class Stacked : public Widget
{

public:
    core::Vec<core::SharedPtr<Widget>> elements;

    ~Stacked() override = default;
    template <typename... T>
    Stacked(T... args)
    {
        elements = core::Vec<core::SharedPtr<Widget>>(args...);
    }

    wgfx::Vec2 preferred_size(wgfx::Vec2 constraint) const override
    {
        wgfx::Vec2 current_constraint = {};
        for (auto &child : childs)
        {
            auto c = child->preferred_size(constraint);
            current_constraint.x = core::max(current_constraint.x, c.x);
            current_constraint.y = core::max(current_constraint.y, c.y);
        }
        return current_constraint;
    }

    wgfx::GRect layout(UiContext const &ctx, wgfx::GRect constraint) override
    {
        for (auto &child : childs)
        {
            child->relayout(ctx, constraint);
        }
        return wgfx::GRect::from_start_end(
            constraint.start.x, constraint.start.y, constraint.end.x, constraint.end.y);
    }

    core::Vec<core::SharedPtr<Widget>> build_childs(UiContext const &v) override
    {
        (void)v;
        return elements;
    };
};
} // namespace fc
