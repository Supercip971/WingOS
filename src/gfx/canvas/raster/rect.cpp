#include "gfx/canvas/cmd.hpp"
#include "rasterCanvas.hpp"

void wgfx::RasterCanvas::rectRoundedFlatAligned(RectCommand const &cmd)
{
    Rgba8 color = cmd.paint.color.toRgba8();
    float roundeness = core::min(cmd.radius, 1.0f);

    float radius = cmd.rect.width() > cmd.rect.height() ? cmd.rect.width() : cmd.rect.height();
    radius *= roundeness / 2.f;
    for (long y = 0; y < cmd.rect.height(); y++)
    {

        // x = r cos ((y / r) * pi/2) for the top left
        // x = width - r cos ((y / r) * pi/2) for the top right
        // x =  r cos (((height-y) / r) * pi/2) for the bottom left
        // x = width - r cos (((height-y) / r) * pi/2) for the bottom right

        float start_x = cmd.rect.start.x;
        float end_x = cmd.rect.end.x;

        // top
        if (y <= radius)
        {
            //  start_x  = radius * cosf(((float)y/radius) * M_PI);
            start_x = radius - sqrtf(radius * radius - (radius - (float)y) * (radius - (float)y));
            // log::log$("start_x: {}\n", (long)start_x);
            end_x = cmd.rect.width() - start_x;
        }
        else if (y >= cmd.rect.height() - radius)
        {

            start_x = radius - sqrtf(radius * radius - (cmd.rect.height() - radius - (float)y) * (cmd.rect.height() - radius - (float)y));
            // log::log$("start_x: {}\n", (long)start_x);
            end_x = cmd.rect.width() - start_x;
        }
        else
        {
            start_x = 0;
            end_x = cmd.rect.width();
        }

        float fs2 = floorf(end_x);
        float fs1 = floorf(start_x);
        for (long x = (long)(start_x + 1.f); x < (long)fs2; x++)
        {
            colorize(x + cmd.rect.start.x, y + cmd.rect.start.y, color);
        }

        blend((long)start_x + cmd.rect.start.x, y + cmd.rect.start.y, color, (1.0f - ((start_x)-fs1)));
        blend((long)end_x + cmd.rect.start.x, y + cmd.rect.start.y, color, ((end_x - fs2)));
    }
}
void wgfx::RasterCanvas::rectFlatAligned(RectCommand const &cmd)
{
    Rgba8 color = cmd.paint.color.toRgba8();
    for (long y = cmd.rect.start.y; y < cmd.rect.end.y; y++)
    {
        for (long x = cmd.rect.start.x; x < cmd.rect.end.y; x++)
        {
            buffer[x + y * width] = color;
        }
    }
}

void wgfx::RasterCanvas::rectStrokeRoundedFlatAligned(RectCommand const &cmd)
{

    Rgba8 color = cmd.paint.color.toRgba8();
    float roundeness = core::min(cmd.radius, 1.0f);

    float w = cmd.paint.stroke.width;
    float sy = -w;
    float ey = w + cmd.rect.height();

    float sx = -w;
    float ex = cmd.rect.width() + w;

    float fh = ey - sy;
    float fw = ex - sx;
    float inner_radius = cmd.rect.width() > cmd.rect.height() ? cmd.rect.width() : cmd.rect.height();

    float outer_radius = fw > fh ? fw : fh;

    outer_radius *= roundeness / 2.f;
    inner_radius = outer_radius - w;
    //  outer_radius += w;

    // top
    for (long y = cmd.rect.start.y + inner_radius + sy; y < cmd.rect.start.y + ey - inner_radius; y++)
    {
        for (long x = sx - w; x < sx; x++)
        {
            buffer[(long)cmd.rect.start.x + x + y * width] = color;
        }

        for (long x = ex; x < ex + w; x++)
        {
            buffer[(long)cmd.rect.start.x + x + y * width] = color;
        }
    }
    for (long x = cmd.rect.start.x + inner_radius + sy; x < cmd.rect.start.x + ex - inner_radius; x++)
    {
        for (long y = sy - w; y < sy; y++)
        {
            buffer[(long) x + (long)(y + cmd.rect.start.y) * width] = color;
        }

        for (long y = ey; y < ey + w; y++)
        {
            buffer[(long) x + (long)(y+cmd.rect.start.y) * width] = color;

        }
    }
    // tops  corner
    for (float y = sy; y < sy + outer_radius; y++)
    {
        float msx = sx;
        float mex = ex;

        msx = outer_radius - sqrtf(outer_radius * outer_radius - (outer_radius - (float)(y + w)) * (outer_radius - (float)(y + w)));

        if (y < 0.f)
        {
            mex = outer_radius + w;
        }
        else
        {

            mex = inner_radius - sqrtf(inner_radius * inner_radius - (inner_radius - (float)(y)) * (inner_radius - (float)(y)));
        }

        msx -= w;
        //        mex -= w;
        //   printf("%f %f\n", msx, mex);

        float fs1 = floorf(msx);
        float fs2 = floorf(mex);
        float absolute_sx = sx + cmd.rect.start.x;

        float absolute_sy = sy + cmd.rect.start.y;

        for (long x = (long)(fs1 + 1.f); x < (long)(fs2); x++)
        {

            // top left
            colorizeChecked(x + absolute_sx, y + absolute_sy, color);
            // top right
            colorizeChecked(fw - x + absolute_sx, y + absolute_sy, color);

            // bottom right
            colorizeChecked(fw - x + absolute_sx, fh - y + absolute_sy, color);

            // bottom left
            colorizeChecked(x + absolute_sx, fh - y + absolute_sy, color);
        }

        blendGamma((long)msx + absolute_sx, y + absolute_sy, color, 1.0f - ((msx - fs1)));
        blendGamma((long)mex + absolute_sx, y + absolute_sy, color, ((mex - fs2)));

        blendGamma((fw - (long)msx + absolute_sx), y + absolute_sy, color, 1.0f - ((msx - fs1)));
        blendGamma((fw - (long)mex + absolute_sx), y + absolute_sy, color, ((mex - fs2)));

        blendGamma((long)msx + absolute_sx, fh - y + absolute_sy, color, 1.0f - ((msx - fs1)));
        blendGamma((long)mex + absolute_sx, fh - y + absolute_sy, color, ((mex - fs2)));

        blendGamma((fw - (long)msx + absolute_sx), fh - y + absolute_sy, color, 1.0f - ((msx - fs1)));
        blendGamma((fw - (long)mex + absolute_sx), fh - y + absolute_sy, color, ((mex - fs2)));
    }
}

void wgfx::RasterCanvas::rectStrokeFlatAligned(RectCommand const &cmd)
{

    Rgba8 color = cmd.paint.color.toRgba8();
    float w = cmd.paint.stroke.width;
    long sy = core::max(cmd.rect.start.y - w, 0);
    long ey = core::min(cmd.rect.end.y + w, (long)height);

    long sx = core::max(cmd.rect.start.x - w, 0);
    long ex = core::min(cmd.rect.end.x + w, (long)width);

    // top
    for (long y = sy; y < sy + w; y++)
    {
        for (long x = sx; x < ex; x++)
        {
            buffer[x + y * width] = color;
        }
    }

    // bottom
    for (long y = ey - w; y < ey; y++)
    {
        for (long x = sx; x < ex; x++)
        {
            buffer[x + y * width] = color;
        }
    }
    // right
    for (long x = ex - w; x < ex; x++)
    {
        for (long y = sy; y < ey; y++)
        {
            buffer[x + y * width] = color;
        }
    }
    // left
    for (long x = sx; x < sx + w; x++)
    {
        for (long y = sy; y < ey; y++)
        {
            buffer[x + y * width] = color;
        }
    }
}

void wgfx::RasterCanvas::rect(RectCommand const &cmd)
{

    if (cmd.paint.type == PAINT_MODE_STROKE)
    {
        if (cmd.radius <= 0.0001f)
        {
            rectStrokeFlatAligned(cmd);
        }
        else
        {
            rectStrokeRoundedFlatAligned(cmd);
        }
        return;
    }
    if (cmd.radius <= 0.0001f)
    {
        rectFlatAligned(cmd);
    }
    else
    {
        rectRoundedFlatAligned(cmd);
    }
}
