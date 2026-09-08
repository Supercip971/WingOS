#pragma once

#include "gfx/canvas/canvas.hpp"
#include "gfx/geometry/rect.hpp"
#include "gfx/geometry/vec2.hpp"
#include "libcore/shared.hpp"
#include "ui/context.hpp"
#include "ui/font-manager.hpp"
#include "ui/widgets/button.hpp"
#include "ui/widgets/centered.hpp"
#include "ui/widgets/container.hpp"
#include "ui/widgets/drageable.hpp"
#include "ui/widgets/padded.hpp"
#include "ui/widgets/text.hpp"
#include "ui/widgets/vflex.hpp"
#include "ui/widgets/widget.hpp"

namespace fc
{

struct WindowWidgetParams
{
    float _width = 0;
    float _height = 0;
    fc::Str _title = "";

    WindowWidgetParams() = default;

    constexpr WindowWidgetParams(float width, float height)
        : _width(width), _height(height)
    {
    }

    constexpr WindowWidgetParams width(float w) const
    {
        WindowWidgetParams s = *this;
        s._width = w;
        return s;
    }

    constexpr WindowWidgetParams height(float h) const
    {
        WindowWidgetParams s = *this;
        s._height = h;
        return s;
    }

    WindowWidgetParams title(fc::Str const &str) const
    {
        WindowWidgetParams s = *this;
        s._title = str;
        return s;
    }
};

class WindowWidget : public Widget
{

public:
    WindowWidgetParams _params;
    fc::SharedPtr<Widget> child;

    ~WindowWidget() override = default;

    WindowWidget() = default;

    WindowWidget(WindowWidgetParams parms)
    {
        _params = parms;
    }

    template <typename T>
    WindowWidget(T args)
    {
        child = (args);
    }

    void insertChild(fc::SharedPtr<Widget> _child) override
    {
        child = _child;
    }

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

        return fc::DrageableContainer(
                   DrageableContainerParams(
                       _params._width,
                       _params._height)
                       .bg(
                           wgfx::CONTAINER_BORDER))
            .vflex()
            .pad(
                fc::Padded().horizontal(16 * ctx.dpi).top(4 * ctx.dpi),
                text)
            .pad(
                fc::Padded().horizontal(2 * ctx.dpi).vertical(2 * ctx.dpi),
                $<Container>(ContainerParms(), child))
            .end();
    };
};

} // namespace fc
