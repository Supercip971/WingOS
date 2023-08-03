#pragma once
#include <libcore/type-utils.hpp>

#include "libcore/result.hpp"

namespace math
{
template <typename T>
class Range
{

private:
    T _start;
    T _end;

public:
    using Type = T;
    Range(){};
    Range(T start, T end) : _start(start), _end(end) {}

    constexpr T start() const { return _start; }

    constexpr T end() const { return _end; }

    constexpr T length() const { return _end - _start; }

    constexpr bool contains(T value) const { return value >= _start && value < _end; }

    constexpr bool contains(Range<T> range) const { return contains(range.start()) && contains(range.end()); }

    constexpr bool overlaps(Range<T> range) const { return contains(range.start()) || contains(range.end()); }

    template <typename C>
    constexpr Range<C> as() const
        requires(core::IsConvertibleTo<T, C>)
    {
        return Range<C>(C(_start), C(_end));
    }

    static constexpr Range<T> from_begin_len(T begin, T len) { return Range<T>(begin, begin + len); }
};

template <typename T>
concept IntRangeable = requires(T a) {
                           {
                               a.start()
                               } -> core::IsIdentityIntegral;
                           {
                               a.end()
                               } -> core::IsIdentityIntegral;
                           {
                               a.length()
                               } -> core::IsIdentityIntegral;
                           {
                               a.contains(a.start())
                               } -> core::IsIdentityIntegral;
                           {
                               a.contains(a)
                               } -> core::IsIdentityIntegral;
                           {
                               a.overlaps(a)
                               } -> core::IsIdentityIntegral;
                       };
} // namespace math