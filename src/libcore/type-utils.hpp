#pragma once

namespace fc
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

// copy
template <class T>
constexpr T copy(T const &t)
{
    return t;
}

} // namespace fc
