#pragma once

#include "gfx/canvas/draw_context.hpp"

#include "gfx/canvas/canvas.hpp"
#include "gfx/canvas/cmd.hpp"
#include "gfx/color.hpp"
#include "gfx/geometry/rect.hpp"
#include "gfx/geometry/shape.hpp"
#include "gfx/geometry/vec2.hpp"
#include "libcore/ds/vec.hpp"
#include "libcore/fmt/log.hpp"
#include "libcore/logic.hpp"
#include "libcore/shared.hpp"
namespace wgfx
{

class RasterCanvas : public wgfx::Canvas
{

    core::Vec<Rgba8> _backdrop_workspace1 = {};
    core::Vec<Rgba8> _backdrop_workspace2 = {};

public:
    Rgba8 *buffer;
    size_t width;
    size_t height;
    size_t bpp;

    inline constexpr void colorizeChecked(long x, long y, Rgba8 col)
    {

        if (x >= 0 && x < (long)width && y >= 0 && y < (long)height)
        {
            buffer[x + y * width] = col;
        }
    }

    inline constexpr void colorize(long x, long y, Rgba8 col)
    {
        buffer[x + y * width] = col;
    }
    inline constexpr void blend(long x, long y, Rgba8 col, float factor)
    {
        buffer[x + y * width].blend(col, factor);
    }

    inline constexpr void blendGamma(long x, long y, Rgba8 col, float factor)
    {

        buffer[x + y * width].blendGamma(col, factor);
    }
    inline constexpr void blendGammaChecked(long x, long y, Rgba8 col, float factor)
    {

        if (x >= size.start.x && x < (long)size.end.x && y >= size.start.y && y < (long)size.end.y)
        {
            buffer[x + y * width].blendGamma(col, factor);
        }
    }
    inline constexpr void blendChecked(long x, long y, Rgba8 col, float factor)
    {
        if (x >= size.start.x && x < (long)size.end.x && y >= size.start.y && y < (long)size.end.y)
        {
            buffer[x + y * width].blend(col, factor);
        }
    }

    void blurArea(GRect area, float factor);

    void boxBlur4(Rgba8 *scl, Rgba8 *dst, long w, long h, float factor);

    void boxBlur4_H(Rgba8 *scl, Rgba8 *dst, long w, long h, float factor);

    void boxBlur4_T(Rgba8 *scl, Rgba8 *dst, long w, long h, float factor);

    // important keywords definition:
    // - flat: unique color for the fill (gradient not valid)
    // - flatx: unique color for the x span (fast aligned fill)
    // - aligned: no rotation applied
    // - fast: quick path with implementation specific details
    void clear(FillCommand const &fill);

    void clearFlat(FillCommand const &fill);

    void rectRoundedFlatAligned(RectCommand const &cmd);

    void rectFlatAligned(RectCommand const &cmd);

    void rectStrokeRoundedFlatAligned(RectCommand const &cmd);

    void rectStrokeFlatAligned(RectCommand const &cmd);

    void rect(RectCommand const &cmd);

    void texture(TextureCommand const &cmd);
    void texturePixelAlignedFlat(TextureCommand const &cmd);

    // https://fr.wikipedia.org/wiki/Algorithme_de_trac%C3%A9_de_segment_de_Xiaolin_Wu

    void pathFillFlat(ContourCommand const &shape, Vec2 off);

    void pathLineFlat(Vec2 start, Vec2 end, Rgba8 color);
    void contour(ContourCommand const &cmd);

    void textFlat(TextCommand const &cmd);
    void text(TextCommand const &cmd)
    {
        textFlat(cmd);
    }

    virtual void apply(DrawContext const &ctx, RenderCommand const &cmd) override;
};
} // namespace wgfx
