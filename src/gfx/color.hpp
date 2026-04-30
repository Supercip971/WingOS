#pragma once
#include <libcore/fmt/fmt.hpp>
#include <stdint.h>

#include "libcore/io/writer.hpp"
#include "libcore/str.hpp"
#define DO_NOT_LOG
#include <libc/math.h>
namespace wgfx
{

class Rgba8
{
public:
    uint8_t b;
    uint8_t g;
    uint8_t r;
    uint8_t a;

    constexpr Rgba8() : b(0), g(0), r(0), a(255) {}
    constexpr Rgba8(uint8_t _r, uint8_t _g, uint8_t _b, uint8_t _a = 255) : b(_b),
                                                                            g(_g),
                                                                            r(_r),
                                                                            a(_a) {};

    // FIXME: verify this, this is an easy quick way to do it
    constexpr Rgba8 blended(Rgba8 to)
    {
        return Rgba8(
            (uint8_t)(((uint16_t)r * a + (uint16_t)to.r * to.a) / (a + to.a)),
            (uint8_t)(((uint16_t)g * a + (uint16_t)to.g * to.a) / (a + to.a)),
            (uint8_t)(((uint16_t)b * a + (uint16_t)to.b * to.a) / (a + to.a)),
            (uint8_t)(((uint16_t)a)));
    }

    constexpr void blend(Rgba8 to)
    {
        r = (uint8_t)(((uint16_t)r * a + (uint16_t)to.r * to.a) / ((uint16_t)a + (uint16_t)to.a));
        g = (uint8_t)(((uint16_t)g * a + (uint16_t)to.g * to.a) / ((uint16_t)a + (uint16_t)to.a));
        b = (uint8_t)(((uint16_t)b * a + (uint16_t)to.b * to.a) / ((uint16_t)a + (uint16_t)to.a));
        a = (uint8_t)(((uint16_t)a));
    }

    constexpr void blendGamma(Rgba8 to, float alpha)
    {
        r = (uint8_t)sqrtf(((float)(r * r) * (1.0f - alpha) + (float)(to.r * to.r) * alpha));
        g = (uint8_t)sqrtf(((float)(g * g) * (1.0f - alpha) + (float)(to.g * to.g) * alpha));
        b = (uint8_t)sqrtf(((float)(b * b) * (1.0f - alpha) + (float)(to.b * to.b) * alpha));
        a = (uint8_t)(((uint16_t)a));
    }

    constexpr void blend(Rgba8 to, float alpha)
    {
        r = (uint8_t)(((float)r * (1.0f - alpha) + (float)to.r * alpha));
        g = (uint8_t)(((float)g * (1.0f - alpha) + (float)to.g * alpha));
        b = (uint8_t)(((float)b * (1.0f - alpha) + (float)to.b * alpha));
        a = (uint8_t)(((uint16_t)a));
    }

    constexpr Rgba8 with_alpha(uint8_t alpha)
    {
        return Rgba8(
            r,
            g,
            b,
            alpha);
    }

    static constexpr Rgba8 from_01a(float r, float g, float b, float a)
    {
        Rgba8 rgba = Rgba8(
            (uint8_t)(r * 255.f),
            (uint8_t)(g * 255.f),
            (uint8_t)(b * 255.f),
            (uint8_t)(a * 255.f));

        return rgba;
    }
} __attribute__((packed));

class Rgba01
{
public:
    float r;
    float g;
    float b;
    float a;

    static constexpr Rgba01 from_01a(float r, float g, float b, float a)
    {
        Rgba01 rgba = {
            .r = r,
            .g = g,
            .b = b,
            .a = a};

        return rgba;
    }

    constexpr Rgba8 toRgba8() const
    {
        return Rgba8::from_01a(r, g, b, a);
    }
};

#define rt_abs(a) (((a) < 0.f) ? -(a) : (a))

#define rt_floor(a) (((a) == (long)(a) || (a) >= 0.f) ? (float)((long)a) : (float)((long)(a) - 1))

constexpr static inline float fast_cos(float x)
{
    /*
    x *= M_1_PI * 0.5;
    x -= 0.25 + rt_floor(x + 0.25);
    x *= 16 * (rt_abs(x) - (0.5));
    return x;
    */
    constexpr auto tp = 1. / (2. * M_PI);
    x *= tp;
    x -= (.25) + rt_floor(x + (.25));
    x *= (16.) * (rt_abs(x) - (.5));
    x += (.225) * x * (rt_abs(x) - (1.));
    return x;
}
constexpr static inline float fast_sin(float x)
{
    return fast_cos(x - M_PI_2);
}

constexpr static inline float fast_sqrt01(float x)
{
    float x_0 = 0.2f;
    float x_1 = 0.5f * (x_0 + (x) / (x_0));
    float x_2 = 0.5f * (x_1 + (x) / (x_1));
    float x_3 = 0.5f * (x_2 + (x) / (x_2));
    float x_4 = 0.5f * (x_3 + (x) / (x_3));

    float x_5 = 0.5f * (x_4 + (x) / (x_4));
    float x_6 = 0.5f * (x_5 + (x) / (x_5));
    return x_6;
}

// Standard sRGB OETF: linear light -> display-ready gamma-encoded value.
// Uses x^(1/2.4), NOT sqrt (x^0.5) – those differ by ~4 % at mid-tones.
constexpr static inline float srgb_linear_to_gamma(float x)
{
    if (x <= 0.0f)
        return 0.0f;
    if (x >= 1.0f)
        return 1.0f;
    if (x <= 0.0031308f)
        return 12.92f * x;
    return 1.055f * fast_sqrt01(x) - 0.055f;
}

class CompositeColor
{
public:
    float lightness;
    float a_green_rediness;
    float b_blue_yelowness;
    float transparency;

    static constexpr CompositeColor fromOklch(float L, float C, float H, float alpha = 1.0f)
    {
        // why +PI/2 ? I don't know, it is not specified anywhere in the spec, nor in the CSS doc
        // but I guess it's now like that
        float H_rad = (H *M_PI) / 180.f + M_PI_2;
        CompositeColor col = {
            .lightness = L,
            .a_green_rediness = C * fast_cos(H_rad),
            .b_blue_yelowness = C * fast_sin(H_rad),
            .transparency = alpha};
        return col;
    }

    constexpr Rgba01 toRgba01() const
    {
        float _l = lightness + 0.3963377774f * a_green_rediness + 0.2158037573f * b_blue_yelowness;
        float _m = lightness - 0.1055613458f * a_green_rediness - 0.0638541728f * b_blue_yelowness;
        float _s = lightness - 0.0894841775f * a_green_rediness - 1.2914855480f * b_blue_yelowness;

        float l = _l * _l * _l;
        float m = _m * _m * _m;
        float s = _s * _s * _s;

        // OKLab -> linear sRGB matrix
        float r = +4.0767416621f * l - 3.3077115913f * m + 0.2309699292f * s;
        float g = -1.2684380046f * l + 2.6097574011f * m - 0.3413193965f * s;
        float b = -0.0041960863f * l - 0.7034186147f * m + 1.7076147010f * s;

        // Apply sRGB gamma correction (linear -> display-ready sRGB) with clamping
        return Rgba01::from_01a(
            srgb_linear_to_gamma(r),
            srgb_linear_to_gamma(g),
            srgb_linear_to_gamma(b),
            transparency);
    }

    constexpr Rgba8 toRgba8() const
    {
        return toRgba01().toRgba8();
    }

    static constexpr CompositeColor lerp(CompositeColor a, CompositeColor b, float f)
    {
        CompositeColor comp = {
            .lightness = a.lightness * (1.f - f) + b.lightness * f,
            .a_green_rediness = a.a_green_rediness * (1.f - f) + b.a_green_rediness * f,
            .b_blue_yelowness = a.b_blue_yelowness * (1.f - f) + b.b_blue_yelowness * f,
            .transparency = a.transparency * (1.f - f) + b.transparency * f,
        };
        return comp;
    }
};

static constexpr CompositeColor RED = CompositeColor::fromOklch(63.7f / 100.f, 0.237, 25.331);
static constexpr CompositeColor ORANGE = CompositeColor::fromOklch(70.5f / 100.f, 0.213, 47.604);
static constexpr CompositeColor AMBER = CompositeColor::fromOklch(76.9f / 100.f, 0.188, 70.08);
static constexpr CompositeColor YELLOW = CompositeColor::fromOklch(79.5f / 100.f, 0.184, 86.047);
static constexpr CompositeColor LIME = CompositeColor::fromOklch(76.8f / 100.f, 0.233, 130.85);
static constexpr CompositeColor GREEN = CompositeColor::fromOklch(72.3f / 100.f, 0.219, 149.579);
static constexpr CompositeColor EMERALD = CompositeColor::fromOklch(69.6f / 100.f, 0.17, 162.48);
static constexpr CompositeColor TEAL = CompositeColor::fromOklch(70.4f / 100.f, 0.14, 182.503);
static constexpr CompositeColor CYAN = CompositeColor::fromOklch(71.5f / 100.f, 0.143, 215.221);
static constexpr CompositeColor SKY = CompositeColor::fromOklch(68.5f / 100.f, 0.169, 237.323);
static constexpr CompositeColor BLUE = CompositeColor::fromOklch(62.3f / 100.f, 0.214, 259.815);
static constexpr CompositeColor INDIGO = CompositeColor::fromOklch(58.5f / 100.f, 0.233, 277.117);
static constexpr CompositeColor VIOLET = CompositeColor::fromOklch(60.6f / 100.f, 0.25, 292.717);
static constexpr CompositeColor PURPLE = CompositeColor::fromOklch(62.7f / 100.f, 0.265, 303.9);
static constexpr CompositeColor FUCHSIA = CompositeColor::fromOklch(66.7f / 100.f, 0.295, 322.15);
static constexpr CompositeColor PINK = CompositeColor::fromOklch(65.6f / 100.f, 0.241, 354.308);
static constexpr CompositeColor ROSE = CompositeColor::fromOklch(64.5f / 100.f, 0.246, 16.439);
static constexpr CompositeColor SLATE_WHITE = CompositeColor::fromOklch(98.4f / 100.f, 0.003, 247.858);
static constexpr CompositeColor SLATE_DARK = CompositeColor::fromOklch(12.9f / 100.f, 0.042, 264.695);

template <typename T>
concept ColorChannelable = requires(T const color) {
    {
        color.r
    };
    {
        color.g
    };
    {
        color.b
    };
    {
        color.a
    };
};
template <ColorChannelable C, core::Writable Targ>
constexpr core::Result<int> format_v(Targ &target, C const &value)
{
    target.write(core::Str("{ "));
    fmt::format_v(target, value.r);
    target.write(core::Str("; "));
    fmt::format_v(target, value.g);
    target.write(core::Str("; "));
    fmt::format_v(target, value.b);
    target.write(core::Str("; "));
    fmt::format_v(target, value.a);
    target.write(core::Str(" }"));
    return 0;
}
template <ColorChannelable C, core::Writable Targ>
constexpr core::Result<int> format_v(Targ &target, fmt::FormatFlags<C> color)
{

    target.write(core::Str("{ "));
    fmt::format_v(target, color.forward_flags(color.value.r));
    target.write(core::Str("; "));
    fmt::format_v(target, color.forward_flags(color.value.g));
    target.write(core::Str("; "));
    fmt::format_v(target, color.forward_flags(color.value.b));
    target.write(core::Str("; "));
    fmt::format_v(target, color.forward_flags(color.value.a));
    target.write(core::Str(" }"));
    return 0;
}

}; // namespace wgfx
; // namespace wgfx
