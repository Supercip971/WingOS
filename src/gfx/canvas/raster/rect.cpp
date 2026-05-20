#include "gfx/canvas/cmd.hpp"
#include "rasterCanvas.hpp"

// static size_t l = 0;
void wgfx::RasterCanvas::rectRoundedFlatAligned(RectCommand const &cmdc)
{

    auto cmd = cmdc;
    cmd.rect.start.x = ceilf(cmd.rect.start.x);
    cmd.rect.start.y = ceilf(cmd.rect.start.y);
    cmd.rect.end.x = floorf(cmd.rect.end.x);
    cmd.rect.end.y = floorf(cmd.rect.end.y);

    Rgba8 color = cmd.paint.color.toRgba8();
    // color.r = l;
    // l++;

    float roundeness = cmd.radius;

    float radius = roundeness;

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

        (void)fs1;
        (void)color;
        blend((long)start_x + cmd.rect.start.x, y + cmd.rect.start.y, color, (1.0f - ((start_x)-fs1)));
        blend((long)end_x + cmd.rect.start.x, y + cmd.rect.start.y, color, ((end_x - fs2)));
    }
}

void wgfx::RasterCanvas::rectFlatAligned(RectCommand const &cmd)
{

    Rgba8 color = cmd.paint.color.toRgba8();

    // color.r = l;
    // l++;
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

    // cmd.rect.start.x += cmd.paint.stroke.width * 2.f;
    // cmd.rect.start.y += cmd.paint.stroke.width * 2.f;
    // cmd.rect.end.x -= cmd.paint.stroke.width * 2.f;
    // cmd.rect.end.y -= cmd.paint.stroke.width * 2.f;
    cmd.rect.start.x = ceilf(cmd.rect.start.x);
    cmd.rect.start.y = ceilf(cmd.rect.start.y);
    cmd.rect.end.x = floorf(cmd.rect.end.x);
    cmd.rect.end.y = floorf(cmd.rect.end.y);

    Rgba8 color = cmd.paint.color.toRgba8();

    float roundeness = cmd.radius;

    float w = floorf(cmd.paint.stroke.width);

    float rw = cmd.rect.width();
    float rh = cmd.rect.height();
    float inner_radius = cmd.rect.width() > cmd.rect.height() ? cmd.rect.width() : cmd.rect.height();

    float outer_radius = roundeness;
    inner_radius = outer_radius - w;

    // inner_radius = floorf(inner_radius);
    // outer_radius = ceilf(outer_radius);
    //  outer_radius += w;

    // top
    for (long y = core::max<long>((cmd.rect.start.y + outer_radius), size.start.y);
         y <= core::min<long>(ceilf(cmd.rect.end.y - outer_radius), size.end.y - 1);
         y++)
    {
        for (long x = core::max<long>(floorf(cmd.rect.start.x), size.start.x); x < core::min<long>((cmd.rect.start.x + w), size.end.x); x++)
        {
            //   blend((long)x, y, color, 0.5f);
            buffer[(long)x + y * width] = color;
        }

        for (long x = core::max<long>((cmd.rect.end.x - w), size.start.x); x < core::min<long>(cmd.rect.end.x, size.end.x); x++)
        {
            //   blend((long)x, y, color, 0.5f);
            buffer[(long)x + y * width] = color;
        }
    }

    for (long x = core::max<long>(cmd.rect.start.x + outer_radius, size.start.x);
         x <= core::min<long>(ceilf(cmd.rect.end.x - outer_radius), size.end.x - 1);
         x++)
    {
        for (long y = core::max<long>(floorf(cmd.rect.start.y), size.start.y); y < core::min<long>((cmd.rect.start.y + w), size.end.y); y++)
        {
            //   blend((long)x, y, color, 0.5f);
            buffer[(long)x + (long)(y)*width] = color;
        }

        for (long y = core::max<long>(cmd.rect.end.y - w, size.start.y); y < core::min<long>(cmd.rect.end.y, size.end.y); y++)
        {
            //   blend((long)x, y, color, 0.5f);
            buffer[(long)x + (long)(y)*width] = color;
        }
    }
    // tops  corner

    for (float y = core::max(0, size.start.y - cmd.rect.start.y); y < core::min((outer_radius), size.end.y - cmd.rect.start.y); y++)
    {

        for (float x = core::max<float>((0), size.start.x - cmd.rect.start.x); x < core::min<float>((outer_radius), size.end.x - cmd.rect.start.x); x++)

        {
            float rel_x = outer_radius - x;
            float rel_y = outer_radius - y;

            float rel_x_f = (outer_radius - x) - 0.9f;
            float rel_y_f = (outer_radius - y);

            float rel_x_c = (outer_radius - x) + 0.9f;
            float rel_y_c = (outer_radius - y);

            // avoid using sqrt
            if (rel_x * rel_x + rel_y * rel_y < outer_radius * outer_radius &&
                rel_x * rel_x + rel_y * rel_y > (inner_radius) * (inner_radius))
            {
                // blend(x + cmd.rect.start.x, y + cmd.rect.start.y, color, 0.5f); // TOP LEFT
                // blend(x + cmd.rect.start.x, rh - y + cmd.rect.start.y, color, 0.5f); // TOP RIGHT
                // blend(rw- (x)  + cmd.rect.start.x, y + cmd.rect.start.y, color, 0.5f); // BOTTOM RIGHT
                // blend(rw- (x)  + cmd.rect.start.x, rh -y + cmd.rect.start.y, color, 0.5f); // BOTTOM LEFT
                colorize(x + cmd.rect.start.x, y + cmd.rect.start.y, color);           // TOP LEFT
                colorize(x + cmd.rect.start.x, rh - y + cmd.rect.start.y, color);      // TOP RIGHT
                colorize(rw - x + cmd.rect.start.x, y + cmd.rect.start.y, color);      // BOTTOM RIGHT
                colorize(rw - x + cmd.rect.start.x, rh - y + cmd.rect.start.y, color); // BOTTOM LEFT
            }
            else if ((rel_x_f * rel_x_f + rel_y_f * rel_y_f) < (outer_radius * outer_radius) &&
                     (rel_x_f * rel_x_f + rel_y_f * rel_y_f) > ((inner_radius) * (inner_radius)))
            {

                //   blendGammaChecked(x + cmd.rect.start.x, y + cmd.rect.start.y, RED.toRgba8(), (rel_x - rel_x_f) * (rel_y - rel_y_f));
                float avg = 0.0f;

                for (float xi = 0.0f; xi <= 1.f; xi += 0.24f)
                {
                    float reli_x = outer_radius - x - xi;
                    float reli_y = outer_radius - y;
                    if (reli_x * reli_x + reli_y * reli_y < outer_radius * outer_radius &&
                        reli_x * reli_x + reli_y * reli_y > (inner_radius) * (inner_radius))
                    {

                        avg += 1.f;
                    }
                }
                blendChecked(x + cmd.rect.start.x, y + cmd.rect.start.y, color, avg / (4.f));
                blendChecked(x + cmd.rect.start.x, cmd.rect.end.y - y, color, avg / (4.f));
                blendChecked(cmd.rect.end.x - x, y + cmd.rect.start.y, color, avg / (4.f));
                blendChecked(cmd.rect.end.x - x, cmd.rect.end.y - y, color, avg / (4.f));
            }
            else if ((rel_x_c * rel_x_c + rel_y_c * rel_y_c) < (outer_radius * outer_radius) &&
                     (rel_x_c * rel_x_c + rel_y_c * rel_y_c) > ((inner_radius) * (inner_radius)))
            {

                //   blendGammaChecked(x + cmd.rect.start.x, y + cmd.rect.start.y, RED.toRgba8(), (rel_x - rel_x_f) * (rel_y - rel_y_f));
                float avg = 0.0f;

                // 0.1 => 0.5 => 0.9
                for (float xi = 0.0f; xi <= 1.f; xi += 0.24f)
                {
                    float reli_x = outer_radius - x + xi;
                    float reli_y = outer_radius - y;
                    if (reli_x * reli_x + reli_y * reli_y < outer_radius * outer_radius &&
                        reli_x * reli_x + reli_y * reli_y > (inner_radius) * (inner_radius))
                    {

                        avg += 1.f;
                    }
                }

                blendChecked(x + cmd.rect.start.x, y + cmd.rect.start.y, color, avg / (4.f));
                blendChecked(x + cmd.rect.start.x, cmd.rect.end.y - y, color, avg / (4.f));
                blendChecked(cmd.rect.end.x - x, y + cmd.rect.start.y, color, avg / (4.f));
                blendChecked(cmd.rect.end.x - x, cmd.rect.end.y - y, color, avg / (4.f));
            }

            // rel_x = inner_radius - (ex - x);
            // rel_y = inner_radius - y;
        }

        //
    }
}

void wgfx::RasterCanvas::rectStrokeFlatAligned(RectCommand const &cmdw)
{

    auto cmd = cmdw;
    Rgba8 color = cmd.paint.color.toRgba8();
    float w = cmd.paint.stroke.width;

    cmd.rect.start.x = ceilf(cmd.rect.start.x);
    cmd.rect.start.y = ceilf(cmd.rect.start.y);
    cmd.rect.end.x = floorf(cmd.rect.end.x);
    cmd.rect.end.y = floorf(cmd.rect.end.y);

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
