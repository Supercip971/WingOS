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
        for (size_t x = 0; x < width; x++)
        {
            for (size_t y = 0; y < height; y++)
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
