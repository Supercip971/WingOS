
#include "gfx/canvas/raster/rasterCanvas.hpp"
void wgfx::RasterCanvas::textFlat(TextCommand const &cmd)
{
    Vec2 pos = cmd.pos;
    for (long i = 0; i < (long)cmd.str.len(); ++i)
    {
        uint32_t chr = cmd.str[i];

        auto const &shape = cmd.font->shapes[chr];
        Vec2 opos = pos;
        opos.y += /*cmd.font->ascent + cmd.font->descent-*/ cmd.font->line_gap;
        opos.x += shape.bearing - shape.ibound.start.x;
        pathFillFlat(ContourCommand{cmd.paint, shape.gfx_contour}, opos);

        if (i < (long)cmd.str.len() - 1)
        {
            pos.x += shape.advance + cmd.font->additionalOffset(chr, cmd.str[i + 1]);
        }
    }
}
