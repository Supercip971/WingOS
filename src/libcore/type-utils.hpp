#pragma once
#include <libcore/type/trait.hpp>
namespace core
{
class NoCopy
{
public:
    NoCopy(NoCopy const &) = delete;
    NoCopy &operator=(NoCopy const &) = delete;
    NoCopy(NoCopy &&) = default;
    NoCopy &operator=(NoCopy &&) = default;
    NoCopy() = default;
};

class NoMove
{
public:
    NoMove(NoMove &&) = delete;
    NoMove &operator=(NoMove &&) = delete;

    NoMove() = default;
};

template <class T>
constexpr T &&forward(RemoveReference<T> &t)
{
    return static_cast<T &&>(t);
}

template <class T>
constexpr T &&forward(RemoveReference<T> &&t)
{
    return static_cast<T &&>(t);
}

template <class T>
constexpr RemoveReference<T> &&move(T &&t)
{
    return static_cast<RemoveReference<T> &&>(t);
}

template <typename T>
constexpr void swap(T &a, T &b)
{
    T c = core::move(a);
    a = core::move(b);
    b = core::move(c);
}

} // namespace core