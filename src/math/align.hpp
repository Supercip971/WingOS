
#pragma once

namespace math
{

template <typename T>
inline constexpr T alignDown(T addr, T align)
{
    return addr & ~(align - 1);
}

template <typename T>
inline constexpr T alignUp(T addr, T align)
{
    return (addr + align - 1) & ~(align - 1);
}

} // namespace math