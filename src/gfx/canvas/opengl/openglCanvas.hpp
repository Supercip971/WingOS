#pragma once

#include "SDL3/SDL_opengl.h"
#include "gfx/canvas/draw_context.hpp"

#include "gfx/canvas/canvas.hpp"
#include "gfx/canvas/cmd.hpp"
#include "gfx/color.hpp"
#include "libcore/fmt/log.hpp"
namespace wgfx
{
class OpenglCanvas : public Canvas
{
public:
    float width;
    float height;

    constexpr float scaled_x(float v)
    {
        return (v) * 2.0 / width - 1.0;
    }

    constexpr float scaled_y(float v)
    {

        return 1.0 - (v) * 2.0 / height;
    }

    void rectCommandExecute(RectCommand const &cmd)
    {

        Rgba01 color = cmd.paint.color.toRgba01();

        glColor4f(color.r, color.g, color.b, color.a);

        glRectf(
            scaled_x(cmd.rect.start.x),
            scaled_y(cmd.rect.start.y),
            scaled_x(cmd.rect.end.x),
            scaled_y(cmd.rect.end.y));
    }
    void clearCommandExecute(FillCommand const &fill)
    {
        Rgba01 color = fill.paint.color.toRgba01();

        glClearColor(color.r, color.g, color.b, color.a);
        glClear(GL_COLOR_BUFFER_BIT);
    }
    virtual void apply(DrawContext const &ctx, RenderCommand const &cmd) override
    {
        (void)ctx;
        switch (cmd.kind)
        {
        case wgfx::RenderCommandKind::RENDER_KIND_FILL:
        {
            clearCommandExecute(cmd.fill);
            break;
        }
        case wgfx::RenderCommandKind::RENDER_KIND_RECT:
        {
            rectCommandExecute(cmd.rect);
            break;
        }
        default:
        {
            log::warn$("Unsupported render command kind: {} for raster backend", (int)cmd.kind);
        }
        }
    }
};
} // namespace wgfx
