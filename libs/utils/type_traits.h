#ifndef TYPE_TRAITS_H
#define TYPE_TRAITS_H
#include <utils/type/integral_constant.h>
namespace utils
{
    // remove reference
    template <typename t>
    struct remove_reference
    {
        using type = t;
    };

    template <typename t>
    struct remove_reference<t &>
    {
        using type = t;
    };

    template <typename t>
    struct remove_reference<t &&>
    {
        using type = t;
    };

    template <typename t>
    using remove_reference_t = typename remove_reference<t>::type;

    // remove const
    template <class T>
    struct remove_const
    {
        typedef T type;
    };

    template <class T>
    struct remove_const<const T>
    {
        typedef T type;
    };

    // remove volatile
    template <class T>
    struct remove_volatile
    {
        typedef T type;
    };

    template <class T>
    struct remove_volatile<volatile T>
    {
        typedef T type;
    };

    // remove pointer
    template <class T>
    struct remove_pointer
    {
        typedef T type;
    };

    template <class T>
    struct remove_pointer<T *>
    {
        typedef T type;
    };

    // is const
    template <typename T>
    struct is_const : public false_type
    {
    };

    template <typename T>
    struct is_const<T const> : public true_type
    {
    };

    // is enum
    template <typename T>
    struct is_enum : public integral_constant<bool, __is_enum(T)>
    {
    };

    // is union
    template <typename T>
    struct is_union : public integral_constant<bool, __is_union(T)>
    {
    };

    // is class
    template <typename T>
    struct is_class : public integral_constant<bool, __is_class(T)>
    {
    };

    // is base of
    template <typename base, typename derived>
    struct is_base_of : public integral_constant<bool, __is_base_of(base, derived)>
    {
    };

    // is_function
    template <typename T>
    struct is_function : public integral_constant<bool, !is_const<const T>::value>
    {
    };

    template <typename T>
    struct is_function<T &>
        : public false_type
    {
    };

    template <typename T>
    struct is_function<T &&>
        : public false_type
    {
    };

    template <typename t>
    constexpr decltype(auto) move(t &&val)
    {
        return static_cast<remove_reference_t<t> &&>(val);
    }

} // namespace utils

#endif // TYPE_TRAITS_H
