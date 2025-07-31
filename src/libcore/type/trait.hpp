#pragma once

namespace core
{

template <class T, T v>
struct IntegralConstant
{
    static constexpr T value = v;
    using type = IntegralConstant;
    using value_type = decltype(value);

    constexpr operator value_type() const { return value; }
    constexpr value_type operator()() const { return value; }
};

using TrueType = IntegralConstant<bool, true>;
using FalseType = IntegralConstant<bool, false>;

template <class A, class B>
struct IsSame_i : core::FalseType
{
};

template <class A>
struct IsSame_i<A, A> : core::TrueType
{
};

template <class A, class B>
concept IsSame = IsSame_i<A, B>::value;

template <class T>
struct RemoveReference_I
{
    typedef T type;
};

template <class T>
struct RemoveReference_I<T &>
{
    typedef T type;
};

template <class T>
struct RemoveReference_I<T &&>
{
    typedef T type;
};
template <class T>
using RemoveReference = typename RemoveReference_I<T>::type;

static_assert(IsSame<int, RemoveReference<int &>>);
static_assert(IsSame<int, RemoveReference<int &&>>);
static_assert(IsSame<int, RemoveReference<int>>);

template <class T>
struct RemoveConst_I
{
    typedef T type;
};

template <class T>
struct RemoveConst_I<const T>
{
    typedef T type;
};

template <class T>
struct RemoveConst_I<T *const>
{
    typedef T *type;
};

template <class T>
using RemoveConst = typename RemoveConst_I<T>::type;

template <class T>
using Pure = RemoveConst<RemoveReference<T>>;

template <class T>
using UnderlyingType = __underlying_type(T);

template <typename From, typename To>
concept IsConvertibleTo = __is_convertible_to(From, To);

template <class T>
concept IsEnum = __is_enum(T);

template <class T>
concept IsIntegral = __is_integral(T);

template <class T>
concept IsIdentityIntegral = __is_integral(RemoveReference<T>);



template<bool condition, class TrueType, class FalseType>
struct Conditional
{
    using type = TrueType;
};

template<class TrueType, class FalseType>
struct Conditional<false, TrueType, FalseType>
{
    using type = FalseType;
};

template<bool condition, class TrueType, class FalseType>
using ConditionalType = typename Conditional<condition, TrueType, FalseType>::type;

template<typename T>
 T declval() noexcept
{
    static_assert(false, "declval not allowed in an evaluated context");
}
} // namespace core