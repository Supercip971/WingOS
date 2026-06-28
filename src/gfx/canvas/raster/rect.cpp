#include "gfx/canvas/cmd.hpp"
#include "gfx/color.hpp"
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

        float a = (float)color.a / 255.f;
        if (color.a > 254)
        {

            for (long x = (long)(start_x + 1.f); x < (long)fs2; x++)
            {
                colorize(x + cmd.rect.start.x, y + cmd.rect.start.y, color);
            }
        }
        else
        {

            for (long x = (long)(start_x + 1.f); x < (long)fs2; x++)
            {
                blend(x + cmd.rect.start.x, y + cmd.rect.start.y, color, (float)a);
            }
        }
        (void)fs1;
        (void)color;
        blend((long)start_x + cmd.rect.start.x, y + cmd.rect.start.y, color, (1.0f - ((start_x)-fs1)) * a);
        blend((long)end_x + cmd.rect.start.x, y + cmd.rect.start.y, color, ((end_x - fs2)) * a);
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

    if (color.a > 254)
    {

        for (long y = sy; y < ey; y++)
        {
            for (long x = sx; x < ex; x++)
            {
                buffer[x + y * width] = color;
            }
        }
    }
    else
    {
        float a = (float)color.a / 255.f;
        for (long y = sy; y < ey; y++)
        {
            for (long x = sx; x < ex; x++)
            {
                blend(x, y, color, a);
            }
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
        for (long x = core::max<long>(floorf(cmd.rect.start.x), size.start.x); x <= core::min<long>((cmd.rect.start.x + w), size.end.x - 1); x++)
        {
            //   blend((long)x, y, color, 0.5f);
            buffer[(long)x + y * width] = color;
        }

        for (long x = core::max<long>(floorf(cmd.rect.end.x - w), size.start.x); x <= core::min<long>(cmd.rect.end.x, size.end.x - 1); x++)
        {
            //   blend((long)x, y, color, 0.5f);
            buffer[(long)x + y * width] = color;
        }
    }

    for (long x = core::max<long>(cmd.rect.start.x + outer_radius, size.start.x);
         x <= core::min<long>(ceilf(cmd.rect.end.x - outer_radius), size.end.x - 1);
         x++)
    {
        for (long y = core::max<long>(floorf(cmd.rect.start.y), size.start.y); y <= core::min<long>((cmd.rect.start.y + w), size.end.y - 1); y++)
        {
            //   blend((long)x, y, color, 0.5f);
            buffer[(long)x + (long)(y)*width] = color;
        }

        for (long y = core::max<long>(cmd.rect.end.y - w, size.start.y); y <= core::min<long>(cmd.rect.end.y, size.end.y - 1); y++)
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

// https://blog.ivank.net/fastest-gaussian-blur.html

void compute_gauss_size(float sigma, float &s1, float &s2, float &s3)
{
    static constexpr int n = 3;

    float ideal = sqrtf((12 * sigma * sigma) / n + 1.f);
    long wl = floorf(ideal);
    if (wl % 2 == 0)
    {
        wl--;
    }
    long wu = wl + 2;

    float mideal = (12 * sigma * sigma - n * wl * wl - 4 * n * wl - 3 * n) / (-4.f * wl - 4);

    float m = roundf(mideal);
    s1 = (0 < m ? wl : wu);
    s2 = (1 < m ? wl : wu);
    s3 = (2 < m ? wl : wu);
}

void wgfx::RasterCanvas::boxBlur4(Rgba8 *scl, Rgba8 *dst, long w, long h, float factor)
{

    for (long x = 0; x < w; x++)
    {
        for (long y = 0; y < h; y++)
        {
            dst[x + y * w] = scl[x + y * w];
        }
    }

    boxBlur4_H(dst, scl, w, h, factor);
    boxBlur4_T(scl, dst, w, h, factor);
}

void wgfx::RasterCanvas::boxBlur4_H(Rgba8 *scl, Rgba8 *dst, long w, long h, float factor)
{
    float iarr = 1.f / (factor + factor + 1.f);
    for (int y = 0; y < h; y++)
    {
        int ty = y * w, ly = ty, ry = ty + factor;
        auto fv = scl[ty], lv = scl[ty + w - 1];
        uint64_t r = (factor + 1) * fv.r, g = (factor + 1) * fv.g, b = (factor + 1) * fv.b, a = (factor + 1) * fv.a;
        for (int x = 0; x < factor; x++)
        {
            auto c = scl[ty + x];
            r += c.r;
            g += c.g;
            b += c.b;
            a += c.a;
        }
        for (int x = 0; x <= factor; x++)
        {
            auto c = scl[ry++];
            r += c.r - fv.r;
            g += c.g - fv.g;
            b += c.b - fv.b;
            a += c.a - fv.a;
            dst[ty++] = Rgba8(r * iarr, g * iarr, b * iarr, a * iarr);
        }
        for (int x = factor + 1; x < w - factor; x++)
        {
            auto c = scl[ry++];
            auto l = scl[ly++];
            r += c.r - l.r;
            g += c.g - l.g;
            b += c.b - l.b;
            a += c.a - l.a;
            dst[ty++] = Rgba8(r * iarr, g * iarr, b * iarr, a * iarr);
        }
        for (int x = w - factor; x < w; x++)
        {
            auto c = scl[ly++];
            r += lv.r - c.r;
            g += lv.g - c.g;
            b += lv.b - c.b;
            a += lv.a - c.a;
            dst[ty++] = Rgba8(r * iarr, g * iarr, b * iarr, a * iarr);
        }
    }
}

void wgfx::RasterCanvas::boxBlur4_T(Rgba8 *scl, Rgba8 *dst, long w, long h, float factor)
{
    float iarr = 1.f / (factor + factor + 1.f);
    for (int x = 0; x < w; x++)
    {
        int tx = x, lx = tx, rx = tx + factor * w;
        auto fv = scl[tx], lv = scl[tx + (h - 1) * w];
        uint64_t r = (factor + 1) * fv.r, g = (factor + 1) * fv.g, b = (factor + 1) * fv.b, a = (factor + 1) * fv.a;
        for (int y = 0; y < factor; y++)
        {
            auto c = scl[tx + y * w];
            r += c.r;
            g += c.g;
            b += c.b;
            a += c.a;
        }
        for (int y = 0; y <= factor; y++)
        {
            auto c = scl[rx];
            r += c.r - fv.r;
            g += c.g - fv.g;
            b += c.b - fv.b;
            a += c.a - fv.a;
            dst[tx] = Rgba8(r * iarr, g * iarr, b * iarr, a * iarr);
            tx += w;
            rx += w;
        }
        for (int y = factor + 1; y < h - factor; y++)
        {
            auto c = scl[rx];
            auto l = scl[lx];
            r += c.r - l.r;
            g += c.g - l.g;
            b += c.b - l.b;
            a += c.a - l.a;
            dst[tx] = Rgba8(r * iarr, g * iarr, b * iarr, a * iarr);
            tx += w;
            lx += w;
            rx += w;
        }
        for (int y = h - factor; y < h; y++)
        {
            auto c = scl[lx];
            r += lv.r - c.r;
            g += lv.g - c.g;
            b += lv.b - c.b;
            a += lv.a - c.a;
            dst[tx] = Rgba8(r * iarr, g * iarr, b * iarr, a * iarr);
            tx += w;
            lx += w;
        }
    }
}

void wgfx::RasterCanvas::blurArea(GRect area, float factor)
{
    _backdrop_workspace1.reserve(width * height);
    _backdrop_workspace2.reserve(width * height);
    float s1, s2, s3;
    compute_gauss_size(factor, s1, s2, s3);

    for (long y = 0; y < area.height(); y++)
    {
        for (long x = 0; x < area.width(); x++)

        {
            auto c = buffer[(x + (long)area.start.x) + (y + (long)area.start.y) * width];
            _backdrop_workspace1[x + y * area.width()] = c;
        }
    }

    boxBlur4(_backdrop_workspace1.data(), _backdrop_workspace2.data(), area.width(), area.height(), (s1 - 1) / 2.f);
    boxBlur4(_backdrop_workspace2.data(), _backdrop_workspace1.data(), area.width(), area.height(), (s2 - 1) / 2.f);
    boxBlur4(_backdrop_workspace1.data(), _backdrop_workspace2.data(), area.width(), area.height(), (s3 - 1) / 2.f);

    for (long y = 0; y < area.height(); y++)
    {
        for (long x = 0; x < area.width(); x++)
        {
            buffer[(x + (long)area.start.x) + (y + (long)area.start.y) * width] = _backdrop_workspace1[x + y * area.width()];
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

        if (cmd.paint.blur >= 1.f)
        {
            blurArea(cmd.rect.intersect(size), cmd.paint.blur);
        }
        rectFlatAligned(cmd);
    }
    else
    {
        if (cmd.paint.blur >= 1.f)
        {
            blurArea(cmd.rect.intersect(size), cmd.paint.blur);
        }

        rectRoundedFlatAligned(cmd);
    }
}
