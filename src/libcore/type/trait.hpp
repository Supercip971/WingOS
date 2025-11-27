#pragma once

#include <type_traits>

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

// Use compiler builtin for underlying type - supported by both GCC and Clang
template <class T>
using UnderlyingType = std::underlying_type<T>::type;


//template <typename From, typename To>
//concept IsConvertibleTo = __is_convertible_to(From, To);

template <class T>
concept IsEnum = __is_enum(T);

// Note: this implementation uses C++20 facilities
template<class T>
struct is_integral : IntegralConstant<bool,
    requires (T t, T* p, void (*f)(T)) // T* parameter excludes reference types
    {
        reinterpret_cast<T>(t); // Exclude class types
        f(0); // Exclude enumeration types
        p + t; // Exclude everything not yet excluded but integral types
    }> {};

template <class T>
concept IsIntegral = is_integral<T>::value;

template <class T>
concept IsIdentityIntegral = IsIntegral<RemoveConst<RemoveReference<T>>>;

template <bool condition, class TrueType, class FalseType>
struct Conditional
{
    using type = TrueType;
};

template <class TrueType, class FalseType>
struct Conditional<false, TrueType, FalseType>
{
    using type = FalseType;
};

template <bool condition, class TrueType, class FalseType>
using ConditionalType = typename Conditional<condition, TrueType, FalseType>::type;

template <typename T>
T declval() noexcept
{
    static_assert(false, "declval not allowed in an evaluated context");
}

namespace IsConvertibleImpl 
{
   template<class T> 
   auto test_returnable(int) -> decltype(
    void(static_cast<T(*)()>(nullptr)), TrueType{}    
); 

    template<class> 
    auto test_returnable(...) -> FalseType;

    template <class From, class To>
    auto test_implicit_convertible(int) -> decltype( 
        void(declval<void(&)(To)>()(declval<From>())), TrueType{}
    );

    template<class, class>
    auto test_implicit_convertible(...) -> FalseType;
}


#ifndef __clang__

template <class From, class To>
struct IsImplicitlyConvertible
    : 
        core::IntegralConstant<bool, 
            decltype(core::IsConvertibleImpl::test_implicit_convertible<From, To>(0))::value
            ||
                (IsSame<From, void> && IsSame<To, void>)>

                {

                };
template<class From, class To>
concept IsConvertibleTo = IsImplicitlyConvertible<From, To>::value;


                #else 

template <typename From, typename To>
concept IsConvertibleTo = __is_convertible_to(From, To);
#endif
 
} // namespace core