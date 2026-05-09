#include "gfx/canvas/cmd.hpp"
#include "rasterCanvas.hpp"

//static size_t l = 0;
void wgfx::RasterCanvas::rectRoundedFlatAligned(RectCommand const &cmd)
{

    Rgba8 color = cmd.paint.color.toRgba8();
    //color.r = l;
    //l++;

    float roundeness = core::min(cmd.radius, 1.0f);

    float radius = cmd.rect.width() > cmd.rect.height() ? cmd.rect.width() : cmd.rect.height();
    radius *= roundeness / 2.f;


    long clip_start_y = core::max(cmd.rect.start.y, size.start.y) - cmd.rect.start.y;
    long clip_end_y = core::min(cmd.rect.end.y, size.end.y) - cmd.rect.start.y;
    for (long y = clip_start_y; y < clip_end_y; y++)
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
            // fmt::log$("start_x: {}\n", (long)start_x);
            end_x = cmd.rect.width() - start_x;
        }
        else if (y >= cmd.rect.height() - radius)
        {

            start_x = radius - sqrtf(radius * radius - (cmd.rect.height() - radius - (float)y) * (cmd.rect.height() - radius - (float)y));
            // fmt::log$("start_x: {}\n", (long)start_x);
            end_x = cmd.rect.width() - start_x;
        }
        else
        {
            start_x = 0;
            end_x = cmd.rect.width();
        }

        start_x = core::max(start_x, size.start.x - cmd.rect.start.x - 1.f);
        end_x = core::min(end_x, size.end.x - cmd.rect.start.x);
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

    //color.r = l;
    //l++;
    long sy = cmd.rect.start.y;
    long ey = cmd.rect.end.y;

    long sx = cmd.rect.start.x;
    long ex = cmd.rect.end.x;
    sy = core::max(sy, 0L);
    ey = core::min(ey, (long)height);
    sx = core::max(sx, 0L);
    ex = core::min(ex, (long)width);

    sy = core::max(sy, size.start.y);
    ey = core::min(ey, size.end.y);
    sx = core::max(sx, size.start.x);
    ex = core::min(ex, size.end.x);

    for (long y = sy; y < ey; y++)
    {
        for (long x = sx; x < ex; x++)
        {
            buffer[x + y * width] = color;
        }
    }
}

void wgfx::RasterCanvas::rectStrokeRoundedFlatAligned(RectCommand const &cmd2)
{


    RectCommand cmd = cmd2;

    cmd.rect.start.x += cmd.paint.stroke.width*2.f;
    cmd.rect.start.y += cmd.paint.stroke.width*2.f;
    cmd.rect.end.x -= cmd.paint.stroke.width*2.f;
    cmd.rect.end.y -= cmd.paint.stroke.width*2.f;
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
    for (long y = core::max(cmd.rect.start.y + inner_radius + sy, size.start.y);
        y < core::min(cmd.rect.start.y + ey - inner_radius, size.end.y);
        y++)
    {
        for (long x = core::max(cmd.rect.start.x + sx - w, size.start.x); x < core::min(cmd.rect.start.x + sx, size.end.x); x++)
        {
            buffer[(long)x + y * width] = color;
        }

        for (long x = core::max(cmd.rect.start.x + ex, size.start.x); x < core::min(cmd.rect.start.x + ex + w, size.end.x); x++)
        {
            buffer[(long) x + y * width] = color;
        }
    }

    for (long x = core::max(cmd.rect.start.x + inner_radius + sx, size.start.x);
        x < core::min(cmd.rect.start.x + ex - inner_radius, size.end.x);
        x++)
    {
        for (long y = core::max(cmd.rect.start.y + sy - w, size.start.y); y < core::min(cmd.rect.start.y + sy, size.end.y); y++)
        {
            buffer[(long)x + (long)(y) * width] = color;
        }

        for (long y = core::max(cmd.rect.start.y + ey, size.start.y); y < core::min(cmd.rect.start.y + ey + w, size.end.y); y++)
        {
            buffer[(long)x + (long)(y) * width] = color;
        }
    }
    // tops  corner

    for (float y = core::max(sy, size.start.y - cmd.rect.start.y); y < core::min(sy + outer_radius, size.end.y - cmd.rect.start.y); y++)
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


        // verify sign
        float fs1 = floorf(msx);
        float fs2 = floorf(mex);
        float absolute_sx = sx + cmd.rect.start.x;

        float absolute_sy = sy + cmd.rect.start.y;

        for (long x = core::max((long)(fs1), size.start.x - cmd.rect.start.x)+1.f; x < core::min((long)(fs2), size.end.x - cmd.rect.start.x); x++)
        {

            // top left
            colorize(x + absolute_sx, y + absolute_sy, color);
            // top right
            colorize(fw - x + absolute_sx, y + absolute_sy, color);

            // bottom right
            colorize(fw - x + absolute_sx, fh - y + absolute_sy, color);

            // bottom left
            colorize(x + absolute_sx, fh - y + absolute_sy, color);
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
