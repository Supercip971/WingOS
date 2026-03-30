#pragma once
#include <stdio.h>
#include <stdlib.h>

#include "gfx/canvas/canvas.hpp"
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

    struct CharacterShape
    {
        char c;
        stbtt_vertex *vertices;
        int num_vertices;
        core::SharedPtr<Contour> gfx_contour;
    };

    core::Vec<CharacterShape> shapes;

    static Contour from_stbtt_vertices(float scale, stbtt_vertex *vertices, int num_vertices)
    {
        Contour contour = {};
        for (int i = 0; i < num_vertices; i++)
        {
            int kind = vertices[i].type;
            switch (kind)
            {
            case STBTT_vmove:
                contour.stroke_move(Vec2(vertices[i].x, vertices[i].y) * scale);
                break;
            case STBTT_vline:
                contour.stroke_point(Vec2(vertices[i].x, vertices[i].y) * scale);
                break;
            case STBTT_vcurve:
                contour.stroke_curve(Vec2(vertices[i].x, vertices[i].y) * scale, Vec2(vertices[i].cx, vertices[i].cy) * scale);
                break;
            case STBTT_vcubic:
                contour.stroke_cubic_curve(Vec2(vertices[i].x, vertices[i].y) * scale, Vec2(vertices[i].cx, vertices[i].cy) * scale, Vec2(vertices[i].cx1, vertices[i].cy1) * scale);
                break;
            default:
                contour.stroke_point(Vec2(vertices[i].x, vertices[i].y) * scale);

                break;
            }
        }
        return contour;
    }

    static core::Result<Font> load_font(core::SharedPtr<Typeface> &from, float height)
    {
        Font fn = {};
        fn._from = from;
        fn.shapes = {};

        fn.height = height;

        float rscale = stbtt_ScaleForPixelHeight(&from->raw, height);
        for (size_t i = 0; i < 127; i++)
        {
            CharacterShape shape = {};
            shape.c = static_cast<char>(i);

            shape.num_vertices = stbtt_GetCodepointShape(&from->raw, i, &shape.vertices);

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

            fn.shapes.push(shape);
        }
        return fn;
    }
};
} // namespace wgfx
