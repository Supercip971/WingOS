
#include "gfx/canvas/raster/rasterCanvas.hpp"

void wgfx::RasterCanvas::texturePixelAlignedFlat(TextureCommand const &cmd)
{
    auto &image = cmd.tex->query_image_rgba8().value();

    for (long y = core::max(cmd.rect.start.y, size.start.y); y < core::min(cmd.rect.end.y, size.end.y); y++)
    {
        float iy = y - cmd.rect.start.y;

        long v = (iy);
        for (long x = core::max(cmd.rect.start.x, size.start.x); x < core::min(cmd.rect.end.x, size.end.x); x++)
        {
            float ix = x - cmd.rect.start.x;
            long u = (ix);

            auto color = image.data()[static_cast<long>(v) * image.width() + static_cast<long>(u)];
            buffer[static_cast<long>(y) * width + static_cast<long>(x)] = color;
        }
    }
}
void wgfx::RasterCanvas::texture(TextureCommand const &cmd)
{
    if (!cmd.tex)
        return;

    if (cmd.tex->query_image_rgba8().has_value())
    {
        auto &image = cmd.tex->query_image_rgba8().value();

        for (long y = core::max(cmd.rect.start.y, size.start.y); y < core::min(cmd.rect.end.y, size.end.y); y++)
        {
            float iy = y - cmd.rect.start.y;

            float v = (iy / (cmd.rect.end.y - cmd.rect.start.y)) * image.height();
            for (long x = core::max(cmd.rect.start.x, size.start.x); x < core::min(cmd.rect.end.x, size.end.x); x++)
            {
                float ix = x - cmd.rect.start.x;
                float u = (ix / (cmd.rect.end.x - cmd.rect.start.x)) * image.width();

                auto color = image.data()[static_cast<long>(v) * image.width() + static_cast<long>(u)];
                buffer[static_cast<long>(y) * width + static_cast<long>(x)] = color;
            }
        }
    }
    else
    {
        fmt::warn$("Unable to query rgba 8 bit image, please preload it with texture.compute_image_rgba8() before using image widget with rasterizer backend");
        return;
    }
}
