#include "gfx/canvas/raster/rasterCanvas.hpp"

void wgfx::RasterCanvas::clearFlat(FillCommand const &cmd)
{
    Rgba8 color = cmd.paint.color.toRgba8();
    for (size_t y = 0; y < height; y++)
    {
        for (size_t x = 0; x < width; x++)
        {
            buffer[x + y * width] = color;
        }
    }
}
void wgfx::RasterCanvas::clear(FillCommand const &cmd)
{
    clearFlat(cmd);
}
