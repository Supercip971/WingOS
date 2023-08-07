#pragma once
#include <libcore/type-utils.hpp>

#include "align.hpp"
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

    constexpr T len() const { return _end - _start; }

    constexpr void start(T v) { _start = v; }

    constexpr void end(T v) { _end = v; }

    constexpr void len(T v) { _end = _start + v; }

    constexpr bool contains(T value) const { return value >= _start && value < _end; }

    constexpr bool contains(Range<T> range) const { return contains(range.start()) && contains(range.end()); }

    constexpr bool overlaps(Range<T> range) const { return contains(range.start()) || contains(range.end()); }

    // lower will be ceiled
    // upper will be floored
    // making the range:
    // -> range.len >= aligned_range.len
    constexpr Range<T> shrinkAlign(size_t alignment) const
    {
        auto aligned_start = math::alignUp(_start, alignment);
        auto aligned_len = math::alignDown(len(), alignment);

        return this->from_begin_len(aligned_start, aligned_len);
    }

    constexpr Range<T> offsetted(T offset) const
    {
        return Range<T>(_start + offset, _end + offset);
    }

    constexpr Range<T> offsettedSub(T offset) const
    {
        return Range<T>(_start - offset, _end - offset);
    }
    constexpr Range<T> div(size_t div) const
    {
        return Range<T>(_start / div, _end / div);
    }

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
                               a.len()
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