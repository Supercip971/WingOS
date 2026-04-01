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

    // https://fr.wikipedia.org/wiki/Algorithme_de_trac%C3%A9_de_segment_de_Xiaolin_Wu

    void drawShapeRaster(ContourCommand const &shape)
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

        //        active.winding = edge.sy > edge.ey ? 1 : -1;

        for (long y = sy; y < ey; y++)
        {
            current.clear();
            //  float draw = 0.0f;

            float y_sample = (float)y + 0.5f;

            for (RawStroke const &line : c->strokes)
            {
                Vec2 start = line.a.pos;

                Vec2 end = line.b.pos;

                start.y = c->bound().end.y - start.y;

                end.y = c->bound().end.y - end.y;

                int winding = start.y > end.y ? 1 : -1;


                // avoid double counting on a single point


                if(y_sample < core::min(start.y, end.y) || y_sample > core::max(start.y, end.y))
                    continue;

                if (core::abs(end.y - start.y) < 0.01f)
                {
                    current.push(
                        RasterLine{
                            .x_pos = core::min(start.x, end.x),
                            .winding = winding,
                        });

                    current.push(
                        RasterLine{
                            .x_pos = core::max(start.x, end.x),
                            .winding = -winding,

                        });
                    continue;
                }

                float raymarched = (((y_sample - start.y) * (end.x - start.x)) / (end.y - start.y));


                current.push(
                    RasterLine{
                        .x_pos = raymarched + start.x,
                        .winding = winding,
                    });
            }


            if(current.len() < 2)
            {
                continue;
            }

            current.quick_sort([](RasterLine const &a, RasterLine const &b)
                               { return (a.x_pos - b.x_pos); }, 0, current.len());

            int winding = 0;
            for (long i = 0; i + 1 < (long)current.len(); i += 1)
            {

                auto s1 = current[i].x_pos;
                auto s2 = current[i + 1].x_pos;


                if(s1 < c->bound().start.x || s2 >= c->bound().end.x)
                    continue;

                winding += current[i].winding;

                if ((winding + current[i].winding) % 2 == 0)
                {

                    for (long x = (long)s1 + 1; x < (long)floor(s2); x++)
                    {
                        colorize(x, y, shape.paint.color.toRgba8());
                    }
                    blendChecked((long)s1, y, shape.paint.color.toRgba8(), 1.0f - (s1 - floorf(s1)));
                    blendChecked((long)s2, y, shape.paint.color.toRgba8(), s2 - floorf(s2));

                }
            }
        }
    }
    void drawLineFast(Vec2 start, Vec2 end, Rgba8 color)
    {

        GRect rect = {0, 0, (long)width, (long)height};

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
            drawShapeRaster(cmd.contour);
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
