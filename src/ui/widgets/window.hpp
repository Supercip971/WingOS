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
#include "ui/font-manager.hpp"
#include "ui/widgets/container.hpp"
#include "ui/widgets/drageable.hpp"
#include "ui/widgets/padded.hpp"
#include "ui/widgets/statefull.hpp"
#include "ui/widgets/text.hpp"
#include "ui/widgets/vflex.hpp"
#include "widget.hpp"

namespace fc
{

struct WindowWidgetParams
{
    wgfx::CompositeColor _bg = wgfx::CONTAINER_FILL;
    wgfx::CompositeColor _border = wgfx::CONTAINER_BORDER;
    float _radius = 16.f;
    float _elevation = 12.0f;
    fc::Str _title = "window";
    float _width = 0;
    float _height = 0;

    constexpr WindowWidgetParams()
    {
    }

    constexpr WindowWidgetParams(
        float initial_width,
        float initial_height

    )
    {
        _width = initial_width;
        _height = initial_height;
    };

    constexpr WindowWidgetParams bg(wgfx::CompositeColor bg) const
    {
        WindowWidgetParams p = *this;
        p._bg = bg;
        return p;
    }

    constexpr WindowWidgetParams border(wgfx::CompositeColor border) const
    {
        WindowWidgetParams p = *this;
        p._border = border;
        return p;
    }

    constexpr WindowWidgetParams radius(float radius) const
    {
        WindowWidgetParams p = *this;
        p._radius = radius;
        return p;
    }

    constexpr WindowWidgetParams elevation(float elevation) const
    {
        WindowWidgetParams p = *this;
        p._elevation = elevation;
        return p;
    }

    constexpr WindowWidgetParams title(fc::Str title) const
    {
        WindowWidgetParams p = *this;
        p._title = title;
        return p;
    }
};

class WindowWidget : public Widget
{

public:
    WindowWidgetParams _params;
    fc::SharedPtr<Widget> child;

    ~WindowWidget() override = default;

    template <typename T>
    WindowWidget(WindowWidgetParams parms, T args)
    {
        _params = parms;
        child = (args);
    }

    fc::SharedPtr<Widget> build(UiContext const &ctx) override
    {

        auto text = $<fc::TextWidget>(_params._title,
                                      fc::FontsRepo::the().find("oswald@32"));

        return $<DrageableContainer>(
            DrageableContainerParams(
                _params._width,
                _params._height)
                .bg(
                    wgfx::CONTAINER_BORDER),
            $<VFlex>(
                $<fc::LPadded>(
                    fc::Padded().horizontal(16 * ctx.dpi).top(4 * ctx.dpi),
                    text),
                $<fc::LPadded>(
                    fc::Padded().horizontal(2 * ctx.dpi).vertical(2 * ctx.dpi), $<Container>(ContainerParms(), child))));
    };
};

} // namespace fc
