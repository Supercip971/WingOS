#pragma once

/*
Logic is some simple functions,
They could be placed somewhere else, such as math.h but
I feel like it doesn't belong there. As it is not really used as math ?
Dunnow
*/

namespace core
{

constexpr static inline auto max(auto a, auto b)
{
    return a > b ? a : b;
}

constexpr static inline auto min(auto a, auto b)
{
    return a < b ? a : b;
}

constexpr static inline auto abs(auto a)
{
    return a < 0 ? -a : a;
}

constexpr static inline auto clamp(auto a, auto min_v, auto max_v)
{
    return max(min_v, min(max_v, a));
}

} // namespace core