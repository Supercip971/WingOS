#pragma once
#include <math.h>

#include "gfx/canvas/draw_context.hpp"

#include "gfx/canvas/canvas.hpp"
#include "gfx/canvas/cmd.hpp"
#include "gfx/color.hpp"
#include "gfx/geometry/shape.hpp"
#include "gfx/geometry/vec2.hpp"
#include "libcore/fmt/log.hpp"
#include "libcore/logic.hpp"
#include "libcore/shared.hpp"
namespace wgfx
{
class RasterCanvas : public wgfx::Canvas
{
public:
    Rgba8 *buffer;
    size_t width;
    size_t height;
    size_t bpp;

    inline constexpr void colorize(long x, long y, Rgba8 col)
    {
        buffer[x + y * width] = col;
    }
    inline constexpr void blend(long x, long y, Rgba8 col, float factor)
    {
        buffer[x + y * width].blend(col, factor);
    }
    inline constexpr void blendChecked(long x, long y, Rgba8 col, float factor)
    {
        if (x >= 0 && x < (long)width && y >= 0 && y < (long)height)
        {
            buffer[x + y * width].blend(col, factor);
        }
    }

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

        for (long y = cmd.rect.start.y; y < cmd.rect.end.y; y++)
        {
            for (long x = cmd.rect.start.x; x < cmd.rect.end.y; x++)
            {
                buffer[x + y * width] = color;
            }
        }
    }

    // https://terathon.com/i3d2018_lengyel.pdf public domain
    constexpr inline Vec2 solvePoly(Vec2 p1, Vec2 p2, Vec2 p3, float y_sample, bool &intersect1, bool &intersect2)
    {
        Vec2 a = p1 - p2 * 2.0f + p3;
        Vec2 b = (p2 - p1) * 2.0f;
        Vec2 c = p1 - Vec2(0.0, y_sample);

        float t1 = 0.0f;
        float t2 = 0.0f;

        if (core::abs(a.y) < 1e-4f)
        {

            if (core::abs(b.y) < 1e-4f)
            {
                return {};
            }
            t1 = (t2 = (-c.y) / (b.y));
            if (t1 < 0.0f || t1 >= 1.0f)
            {
                return {};
            }

            if (core::abs(t1 - 1.0f) < 0.0001f)
            {
                intersect1 = false;
            }
            else
            {
                intersect1 = true;
            }
        }
        else
        {

            float dis = b.y * b.y - 4.f * a.y * (c.y);

            if (dis < -0.000001f)
            {
                return {};
            }
            float rt = sqrtf(core::max(dis, 0.0f));

            t1 = (-b.y - rt) / (2.f * a.y);
            t2 = (-b.y + rt) / (2.f * a.y);

            if (t1 >= 0.0f && t1 < 1.0f)
            {
                intersect1 = true;
            }

            if (t2 >= 0.0f && t2 < 1.0f)
            {
                intersect2 = true;
            }
        }

        // if the derivative, aka:
        // a t² + bt + c
        // 2 a t + b < 0
        // then we need to swap t1, t2

        return Vec2(a.x * t1 * t1 + b.x * t1 + c.x, a.x * t2 * t2 + b.x * t2 + c.x);
    }
    // https://fr.wikipedia.org/wiki/Algorithme_de_trac%C3%A9_de_segment_de_Xiaolin_Wu

    void drawShapeRaster(ContourCommand const &shape, Vec2 off)
    {
        core::SharedPtr<Contour> const &c = shape.contour;

        long sy = core::max(floor(c->bound().start.y) - 1, 0);
        long ey = core::min(ceil(c->bound().end.y) + 1, height - 1);

        struct RasterLine
        {
            float x_pos;
            int winding;
        };

        core::Vec<RasterLine> current = {};
        current.reserve(c->strokes.len());

        for (long y = sy; y < ey; y++)
        {
            current.clear();
            //  float draw = 0.0f;

            float y_sample = (float)y + 0.5f;

            // critical loop
            for (RawStroke const &line : c->strokes)
            {

                Vec2 p1 = line.a.pos;
                Vec2 p3 = line.b.pos;
                Vec2 p2;

                bool isDownward = p1.y > p3.y;
                p3.y = (c->bound().end.y - p3.y);
                p1.y = (c->bound().end.y - p1.y);
                if (isDownward)
                {
                    if (p1.y <= y_sample && p3.y < y_sample)
                        continue;
                    if (p1.y >= y_sample && p3.y > y_sample)
                        continue;
                }
                else
                {
                    if (p1.y < y_sample && p3.y <= y_sample)
                        continue;
                    if (p1.y > y_sample && p3.y >= y_sample)
                        continue;
                }
                if (line.b.action == PathAction::GCURVE)
                {
                    p2 = (line.b.curve.control);
                }
                else
                {
                    p2 = (p1 + p3) / 2.0f;
                }

                p2.y = (c->bound().end.y - p2.y);

                bool t1 = false;
                bool t2 = false;
                auto res = solvePoly(p1, p2, p3, y_sample, t1, t2);

                int n = isDownward ? 1 : -1;
                if (t1)
                {
                    current.push(
                        RasterLine{
                            .x_pos = res.x,
                            .winding = 1 * n,
                        });
                }
                if (t2)
                {

                    current.push(
                        RasterLine{
                            .x_pos = res.y,
                            .winding = -1 * n,
                        });
                }
            }

            if (current.len() < 2)
            {
                continue;
            }

            // see why miracly removing this quicksort don't break everything
            current.quick_sort([](RasterLine const &a, RasterLine const &b)
                               { return (a.x_pos - b.x_pos); }, 0, current.len());

            int winding = 0;
            for (long i = 0; i + 1 < (long)current.len(); i += 1)
            {
                winding += current[i].winding;

                if (winding % 2 != 0)
                {

                    auto s1 = current[i].x_pos;
                    auto s2 = current[i + 1].x_pos;

                    s1 = core::clamp(s1, c->bound().start.x, c->bound().end.x);
                    s2 = core::clamp(s2, c->bound().start.x, c->bound().end.x);

                    float fs2 = floorf(s2);
                    float fs1 = floorf(s1);
                    for (long x = (long)(s1 + 1.f); x < (long)fs2; x++)
                    {
                        colorize(x + off.x, y + off.y, shape.paint.color.toRgba8());
                    }

                    blendChecked((long)s1 + off.x, y + off.y, shape.paint.color.toRgba8(), 1.0f - ((s1)-fs1));
                    blendChecked((long)s2 + off.x, y + off.y, shape.paint.color.toRgba8(), (s2 - fs2));
                }
            }
        }
    }
    void drawLineFast(Vec2 start, Vec2 end, Rgba8 color)
    {

        GRect rect = {0, 0, (float)width, (float)height};

        start = rect.contained(start);
        end = rect.contained(end);

        //        int x0 = start.x, y0 = start.y;
        //       int x1 = end.x, y1 = end.y;

        // Find the greater difference
        auto delta = end - start;
        int steep = abs(delta.y) > abs(delta.x);

        // Swap x and y if y has a greater difference than x
        if (steep)
        {
            core::swap(end.x, end.y);
            core::swap(start.x, start.y);
        }
        // Set the smaller x value to x0
        if (start.x > end.x)
        {
            core::swap(start, end);
        }
        delta = end - start;

        float gradient = (fabs(delta.x) <= 0.001) ? 1.0f : delta.y / delta.x;
        // First endpoint
        float xend = floor(start.x);
        float yend = start.y + gradient * (xend - start.x);
        float xgap = 1.0f - ((start.x) - xend);
        int xpxl1 = xend;
        int ypxl1 = floor(yend);

        if (steep)
        {
            blendChecked(ypxl1, xpxl1, color, (1.0f - (yend - floor(yend))) * xgap);
            blendChecked(ypxl1 + 1, xpxl1, color, (yend - floor(yend)) * xgap);
        }
        else
        {

            blendChecked(xpxl1, ypxl1, color, (1.0f - (yend - floor(yend))) * xgap);
            blendChecked(xpxl1, ypxl1 + 1, color, (yend - floor(yend)) * xgap);
        }

        float intery = yend + gradient;

        // Second endpoint
        xend = ceil(end.x);
        yend = end.y + gradient * (xend - end.x);
        xgap = 1.f - (xend - end.x);
        int xpxl2 = xend;
        int ypxl2 = floor(yend);
        if (steep)
        {
            blendChecked(ypxl2, xpxl2, color, (1.0f - (yend - floor(yend))) * xgap);
            blendChecked(ypxl2 + 1, xpxl2, color, (yend - floor(yend)) * xgap);
        }
        else
        {
            blendChecked(xpxl2, ypxl2, color, (1.0f - (yend - floor(yend))) * xgap);
            blendChecked(xpxl2, ypxl2 + 1, color, (yend - floor(yend)) * xgap);
        }

        // Move between endpoints
        if (steep)
        {

            xpxl1 = core::clamp(xpxl1, rect.start.x, rect.end.x - 1);
            xpxl2 = core::clamp(xpxl2, rect.start.y, rect.end.y - 1);
            intery = core::clamp(intery, rect.start.x, rect.end.x - 1);
        }
        else
        {
            xpxl1 = core::clamp(xpxl1, rect.start.x, rect.end.x - 1);
            xpxl2 = core::clamp(xpxl2, rect.start.x, rect.end.x - 1);
            intery = core::clamp(intery, rect.start.y, rect.end.y - 2);
        }
        for (int x = xpxl1 + 1; x < xpxl2; x++)
        {
            if (steep)
            {
                blend(floor(intery), x, color, (1.0f - (intery - floor(intery))));
                blend(floor(intery) + 1, x, color, (intery - floor(intery)));
            }
            else
            {
                blend(x, floor(intery), color, (1.0f - (intery - floor(intery))));
                blend(x, floor(intery) + 1, color, (intery - floor(intery)));
            }
            intery += gradient;
        }
    }

    void contourCommandExecute(ContourCommand const &cmd)
    {
        Vec2 pos = {};
        for (auto const &stroke : cmd.contour->commands)
        {
            //            log::log$("stroke: {}, {} {}", (int)stroke.pos.x, (int)stroke.pos.y, (int)stroke.action);

            Vec2 ppos = stroke.pos /*+ off*/;
            ppos.y = cmd.contour->bound().end.y - ppos.y;
            switch (stroke.action)
            {
            case PathAction::GMOVE:
                pos = ppos;
                break;
            case PathAction::GPOINT:
                drawLineFast(pos, ppos, cmd.paint.color.toRgba8());
                pos = ppos;
                break;
            case PathAction::GCURVE:
                drawLineFast(pos, ppos, cmd.paint.color.toRgba8());
                pos = ppos;
                break;
            case PathAction::GCUBIC_CURVE:
                drawLineFast(pos, ppos, cmd.paint.color.toRgba8());
                pos = ppos;
                break;
            default:
                log::log$("unknown stroke action: {}", (int)stroke.action);
                break;
            }
        }
    }

    void drawText(TextCommand const &cmd)
    {
        Vec2 pos = cmd.pos;
        for (long i = 0; i < (long)cmd.str.len(); ++i)
        {
            uint32_t chr = cmd.str[i];

            auto const &shape = cmd.font->shapes[chr];
            Vec2 opos = pos;
            opos.y += cmd.font->ascent - cmd.font->descent + cmd.font->line_gap;
            opos.y -=  shape.ibound.end.y;
            opos.x += shape.bearing - shape.ibound.start.x;
            drawShapeRaster(ContourCommand{cmd.paint, shape.gfx_contour}, opos);

            if(i < (long)cmd.str.len() - 1){
                pos.x += shape.advance + cmd.font->additionalOffset(chr, cmd.str[i + 1]);
            }
        }
    }

    virtual void apply(DrawContext const &ctx, RenderCommand const &cmd) override
    {
        (void)ctx;
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
        case wgfx::RenderCommandKind::RENDER_KIND_CONTOUR:
        {
            drawShapeRaster(cmd.contour, cmd.contour.pos);
            break;
        }
        case wgfx::RenderCommandKind::RENDER_KIND_TEXT:
        {
            drawText(cmd.text);
            break;
        }

        default:
        {
            log::warn$("Unsupported render command kind: {} for raster backend", (int)cmd.kind);
        }
        }
    }
};
} // namespace wgfx
