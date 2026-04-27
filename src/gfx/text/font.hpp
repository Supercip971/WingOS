#pragma once
#include <stdio.h>
#include <stdlib.h>

#include "gfx/canvas/canvas.hpp"
#include "gfx/canvas/cmd.hpp"
#include "gfx/geometry/shape.hpp"
#include "libcore/fmt/log.hpp"
#include "libcore/result.hpp"
#include "libcore/shared.hpp"
namespace wgfx
{

class Typeface
{
public:
    uint8_t *buffer;
    stbtt_fontinfo raw;

    static core::Result<core::SharedPtr<Typeface>> from_file(core::Str path)
    {
        core::SharedPtr<Typeface> result = core::SharedPtr<Typeface>::make();
        FILE *f = fopen(path._data, "rb");

        fseek(f, 0, SEEK_END);
        size_t file_size = ftell(f);
        fseek(f, 0, SEEK_SET);

        log::log$("loading font from {} with size: {}", path, file_size);
        result->buffer = (uint8_t *)new uint8_t[file_size];
        fread(result->buffer, 1, file_size, f);
        fclose(f);
        stbtt_InitFont(&result->raw, result->buffer, 0);

        return result;
    }
};

class Font
{
public:
    core::SharedPtr<Typeface> _from;
    float height;
    float weight;
    float rscale;

    float ascent;
    float descent;
    float line_gap;

    wgfx::GRect global_box;

    struct CharacterShape
    {
        char c;
        stbtt_vertex *vertices;
        int num_vertices;
        core::SharedPtr<Contour> gfx_contour;
        wgfx::GRect ibound;
        float advance;
        float bearing;
    };

    core::Vec<CharacterShape> shapes;

    static Contour from_stbtt_vertices(float scale, stbtt_vertex *vertices, int num_vertices)
    {
        Contour contour = {};
        for (int i = 0; i < num_vertices; i++)
        {


            int kind = vertices[i].type;
//#error ascent is incorrect here
            switch (kind)
            {
            case STBTT_vmove:
                contour.stroke_move(Vec2(vertices[i].x, -vertices[i].y) * scale);
                break;
            case STBTT_vline:
                contour.stroke_point(Vec2(vertices[i].x, - vertices[i].y) * scale);
                break;
            case STBTT_vcurve:
                contour.stroke_curve(Vec2(vertices[i].x,  -vertices[i].y) * scale, Vec2(vertices[i].cx,  -vertices[i].cy) * scale);
                break;
            case STBTT_vcubic:
                contour.stroke_cubic_curve(Vec2(vertices[i].x,  -vertices[i].y) * scale, Vec2(vertices[i].cx, -vertices[i].cy) * scale, Vec2(vertices[i].cx1,  -vertices[i].cy1) * scale);
                break;
            default:
                contour.stroke_point(Vec2(vertices[i].x,  -vertices[i].y) * scale);


                break;
            }
        }
        return contour;
    }

    float additionalOffset(int a, int b) const
    {
        return stbtt_GetCodepointKernAdvance(&_from->raw, a, b) * rscale;
    }
    static core::Result<Font> load_font(core::SharedPtr<Typeface> &from, float height)
    {
        Font fn = {};
        fn._from = from;
        fn.shapes = {};

        fn.height = height;

        float rscale = stbtt_ScaleForPixelHeight(&from->raw, height);

        fn.rscale = rscale;
        int ascent = 0;
	(void)ascent;
        int descent = 0;
        int line_gap = 0;
        stbtt_GetFontVMetrics(&from->raw, &ascent, &descent, &line_gap);
        fn.ascent = ascent * rscale;
        fn.descent = descent * rscale;
        fn.line_gap = line_gap * rscale;

        for (size_t i = 0; i < 127; i++)
        {

            CharacterShape shape = {};
            shape.c = static_cast<char>(i);

            shape.num_vertices = stbtt_GetCodepointShape(&from->raw, i, &shape.vertices);

            //     stbtt_GetGlyphHMetrics(const stbtt_fontinfo *info, int glyph_index, int *advanceWidth, int *leftSideBearing)
            int sx = 0;
            int sy = 0;
            int ex = 0;
            int ey = 0;
            stbtt_GetCodepointBox(&from->raw, i, &sx, &sy, &ex, &ey);

            shape.ibound = wgfx::GRect::from_start_end((float)sx * rscale, (float)sy * rscale, (float)ex * rscale, (float)ey * rscale);

            int advance;
            int left_side_bearing;
            stbtt_GetCodepointHMetrics(&from->raw, i, &advance, &left_side_bearing);
            shape.advance = advance * rscale;
            shape.bearing = left_side_bearing * rscale;
            if (shape.num_vertices > 0)
            {
                char str[3] = {shape.c, 0};
                log::log$("loading font: char {}", str);
                shape.gfx_contour = core::SharedPtr<Contour>::make(Font::from_stbtt_vertices(rscale, shape.vertices, shape.num_vertices));
            }
            else
            {
                shape.gfx_contour = core::SharedPtr<Contour>::make();
            }
            //            shape.gfx_contour->bound(GRect::from_start_end((sx-1) * rscale, (sy-1) * rscale,  (ex+1)*rscale,  (ey+1) * rscale));
            fn.shapes.push(shape);
        }
        return fn;
    }
};
} // namespace wgfx
