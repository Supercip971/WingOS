#pragma once

#include "libcore/type-utils.hpp"
namespace fmt
{

// colors, with their correspondign foreground ANSI code
enum class Color
{
    NONE = 0,
    BLACK = 30,
    RED = 31,
    GREEN = 32,
    YELLOW = 33,

    BLUE = 34,
    MAGENTA = 35,
    CYAN = 36,
    WHITE = 37,

    BRIGHT_BLACK = 90,
    BRIGHT_RED = 91,
    BRIGHT_GREEN = 92,
    BRIGHT_YELLOW = 93,
};

template <typename T>
struct FormatFlags
{
    bool is_hex;
    Color color;
    char pad_char = ' ';
    int pad_size = 0;
    T &&value;

    template <typename C>
    constexpr FormatFlags<C> forward_flags(C &&new_value)
    {
        return FormatFlags<C>{
            is_hex,
            color,
            pad_char,
            pad_size,
            core::forward<C>(new_value)};
    }
};

typedef enum
{
    FMT_HEX,
    FMT_CYAN, // [cyan supremacy >:(] no other color for now as this is not a priority
    FMT_PAD_8BYTES,
    FMT_PAD_ZERO,
} FmtFlag;

template <typename T>
constexpr FormatFlags<T> operator|(T &&a, FmtFlag flag)
{
    switch (flag)
    {
    case FMT_PAD_8BYTES:
        return FormatFlags<T>{
            .pad_char = ' ',
            .pad_size = 8,
            .value = core::forward<T>(a),
        };
    case FMT_PAD_ZERO:
        return FormatFlags<T>{
            .pad_char = '0',
            .pad_size = 8,
            .value = core::forward<T>(a),
        };
    case FMT_CYAN:
        return FormatFlags<T>{
            .color = Color::CYAN,
            .value = core::forward<T>(a),
        };
    case FMT_HEX:
        return FormatFlags<T>{

            .is_hex = true,
            .color = Color::NONE,
            .value = core::forward<T>(a),
        };
    }
    return FormatFlags<T>{
        .is_hex = false,
        .value = core::forward<T>(a),
    };
}
template <typename T>
constexpr FormatFlags<T> operator|(const FormatFlags<T> &a, FmtFlag flag)
{
    FormatFlags<T> b = a;
    switch (flag)
    {
    case FMT_PAD_8BYTES:
        b.pad_size = 16;
        break;
    case FMT_PAD_ZERO:
        b.pad_char = '0';
        break;
    case FMT_CYAN:
        b.color = Color::CYAN;
        break;
    case FMT_HEX:
        b.is_hex = true;
        break;
    }
    return core::forward<FormatFlags<T>>(b);
}

template <typename T>
constexpr FormatFlags<T> operator|(FormatFlags<T> &&a, FmtFlag flag)
{
    switch (flag)
    {
    case FMT_PAD_8BYTES:
        a.pad_size = 16;
        break;
    case FMT_PAD_ZERO:
        a.pad_char = '0';
        break;
    case FMT_CYAN:
        a.color = Color::CYAN;
        break;
    case FMT_HEX:
        a.is_hex = true;
        break;
    }
    return core::forward<FormatFlags<T>>(a);
}

} // namespace fmt