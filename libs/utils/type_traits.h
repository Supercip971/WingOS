#ifndef TYPE_TRAITS_H
#define TYPE_TRAITS_H
#include <utils/type/integral_constant.h>
namespace utils
{

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

    template <typename T>
    struct is_enum : public integral_constant<bool, __is_enum(T)>
    {
    };

    template <typename T>
    struct is_union : public integral_constant<bool, __is_union(T)>
    {
    };

    template <typename T>
    struct is_class : public integral_constant<bool, __is_class(T)>
    {
    };

    template <typename t>
    constexpr decltype(auto) move(t &&val)
    {
        return static_cast<remove_reference_t<t> &&>(val);
    }

} // namespace utils

#endif // TYPE_TRAITS_H
