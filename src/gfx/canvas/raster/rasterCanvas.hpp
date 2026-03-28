#pragma once

#include "gfx/canvas/canvas.hpp"
#include "gfx/canvas/cmd.hpp"
#include "gfx/color.hpp"
#include "libcore/fmt/log.hpp"
namespace wgfx
{
class RasterCanvas : public Canvas
{
public:
    Rgba8 *buffer;
    size_t width;
    size_t height;
    size_t bpp;

    void clearCommandExecute(FillCommand const &fill)
    {
        Rgba8 color = fill.paint.color.toRgba8();
        for (size_t y = 0; y < height; y++)
        {
            for (size_t x = 0; x < width; x++)
            {
                buffer[x + y * width] = color;
            }
        }
    }

    void rectCommandExecute(RectCommand const &cmd)
    {
        Rgba8 color = cmd.paint.color.toRgba8();

        for (long y = cmd.rect.y; y < cmd.rect.endy(); y++)
        {
            for (long x = cmd.rect.x; x < cmd.rect.endx(); x++)
            {
                buffer[x + y * width] = color;
            }
        }
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
        commands.clear();
    }
};
} // namespace wgfx
