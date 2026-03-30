#pragma once
#include <stdint.h>
#define DO_NOT_LOG
#include <libc/math.h>
namespace wgfx
{

class Rgba8
{
public:
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;


    // FIXME: verify this, this is an easy quick way to do it
    constexpr Rgba8 blended(Rgba8 to)
    {
        return Rgba8{
            .r = (uint8_t)(((uint16_t)r * a + (uint16_t)to.r * to.a) / (a + to.a)),
            .g = (uint8_t)(((uint16_t)g * a + (uint16_t)to.g * to.a) / (a + to.a)),
            .b = (uint8_t)(((uint16_t)b * a + (uint16_t)to.b * to.a) / (a + to.a)),
            .a = (uint8_t)(((uint16_t)a ))};
    }

    constexpr void blend(Rgba8 to)
    {
        r = (uint8_t)(((uint16_t)r * a + (uint16_t)to.r * to.a) / (a + to.a));
        g = (uint8_t)(((uint16_t)g * a + (uint16_t)to.g * to.a) / (a + to.a));
        b = (uint8_t)(((uint16_t)b * a + (uint16_t)to.b * to.a) / (a + to.a));
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
        return Rgba8{
            .r = r,
            .g = g,
            .b = b,
            .a = alpha};
    }

    static constexpr Rgba8 from_01a(float r, float g, float b, float a)
    {
        Rgba8 rgba = {
            .r = (uint8_t)(r * 255.f),
            .g = (uint8_t)(g * 255.f),
            .b = (uint8_t)(b * 255.f),
            .a = (uint8_t)(a * 255.f)};

        return rgba;
    }
};

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

    constexpr Rgba8 toRgba8() const{
        return Rgba8::from_01a(r,  g,  b,  a);
    }
};

#define rt_abs(a) (((a) < 0) ? -(a) : (a))

#define rt_floor(a) ((a == (long)a || a >= 0) ? (float)((long)a) : (float)((long)a - 1))

constexpr static inline float fast_sin(float x)
{
    x *= M_1_PI * 0.5;
    x -= 0.25 + rt_floor(x + 0.25);
    x *= 16 * (rt_abs(x) - (0.5));
    return x;
}
constexpr static inline float fast_cos(float x)
{
    return fast_sin(M_PI_2 + x);
}

class CompositeColor
{
public:
    float lightness;
    float a_green_rediness;
    float b_blue_yelowness;
    float transparency;

    static constexpr CompositeColor fromOklch(float L, float C, float H, float alpha = 1.0)
    {
        CompositeColor col = {
            .lightness = L,
            .a_green_rediness = C * fast_cos(H),
            .b_blue_yelowness = C * fast_sin(H),
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

        return Rgba01::from_01a(
            +4.0767416621f * l - 3.3077115913f * m + 0.2309699292f * s,
            -1.2684380046f * l + 2.6097574011f * m - 0.3413193965f * s,
            -0.0041960863f * l - 0.7034186147f * m + 1.7076147010f * s,
            transparency);
    }

    constexpr Rgba8 toRgba8() const {
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

static constexpr CompositeColor RED = CompositeColor::fromOklch(63.7 / 100, 0.237, 25.331);

static constexpr CompositeColor INDIGO = CompositeColor::fromOklch(58.5 / 100, 0.233, 277.117);

}; // namespace wgfx
