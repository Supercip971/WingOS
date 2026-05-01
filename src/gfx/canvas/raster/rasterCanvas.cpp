#include "gfx/canvas/raster/rasterCanvas.hpp"

void wgfx::RasterCanvas::apply(DrawContext const &ctx, RenderCommand const &cmd)
{
    (void)ctx;
    switch (cmd.kind)
    {
    case wgfx::RenderCommandKind::RENDER_KIND_FILL:
    {
        clear(cmd.fill);
        break;
    }
    case wgfx::RenderCommandKind::RENDER_KIND_RECT:
    {
        rect(cmd.rect);
        break;
    }
    case wgfx::RenderCommandKind::RENDER_KIND_CONTOUR:
    {
        pathFillFlat(cmd.contour, cmd.contour.pos);
        break;
    }
    case wgfx::RenderCommandKind::RENDER_KIND_TEXT:
    {
        text(cmd.text);
        break;
    }

    default:
    {
        log::warn$("Unsupported render command kind: {} for raster backend", (int)cmd.kind);
    }
    }
}
