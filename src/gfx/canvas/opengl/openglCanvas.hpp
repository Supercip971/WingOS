#pragma once

#include "gfx/canvas/canvas.hpp"
#include "gfx/canvas/cmd.hpp"
#include "gfx/color.hpp"
#include "libcore/fmt/log.hpp"
#include "SDL3/SDL_opengl.h"
namespace wgfx
{
class OpenglCanvas : public Canvas
{
public:
    size_t width;
    size_t height;

    void clearCommandExecute(FillCommand const &fill)
    {
        Rgba01 color = fill.paint.color.toRgba01();

        glClearColor(color.r, color.g, color.b, color.a);
        glClear(GL_COLOR_BUFFER_BIT);
    }
    virtual void apply() override
    {
        for (size_t i = 0; i < commands.len(); i++)
        {
            auto const &cmd = commands[i];
            switch (cmd.kind)
            {
            case wgfx::RenderCommandKind::RENDER_KIND_FILL:
            {
                clearCommandExecute(cmd.fill);
                break;
            }
            default:
            {
                log::warn$("Unsupported render command kind: {} for raster backend", (int)cmd.kind);
            }
            }
        }
        commands.clear();
    }
};
} // namespace wgfx
