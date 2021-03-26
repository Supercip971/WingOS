
#include "type_trait_check.h"
#include <utils/type/integral_constant.h>
#include <utils/type/is_same.h>
#include <utils/type_traits.h>
int integral_constant_check()
{
    utils::integral_constant<int, 0> test1;
    utils::integral_constant<int, 5> test2;
    utils::integral_constant<bool, false> test3;

    if (test1.value != 0)
    {
        return -1;
    }
    if (test2.value != 5)
    {
        return -2;
    }
    if (test3.value != false)
    {
        return -3;
    }
    if (test3)
    {
        return -4;
    }
    if (test3())
    {
        return -5;
    }
    return 0;
}
int integral_constant_false_check()
{
    if (utils::false_type())
    {
        return -2;
    }
    return 0;
}
int integral_constant_true_check()
{
    if (!utils::true_type())
    {
        return -1;
    }
    return 0;
}

int is_same_check_true()
{
    if (!utils::is_same<int, int>())
    {
        return -1;
    }
    if (!utils::is_same<bool, bool>())
    {
        return -2;
    }
    if (!utils::is_same<const char &, const char &>())
    {
        return -3;
    }
    return 0;
}
int is_same_check_false()
{
    if (utils::is_same<int, char>())
    {
        return -1;
    }
    if (utils::is_same<bool, const bool>())
    {
        return -2;
    }
    if (utils::is_same<const char, const char &>())
    {
        return -3;
    }
    return 0;
}

int remove_reference_check()
{
    if (!utils::is_same<char, utils::remove_reference<char &>::type>())
    {
        return -1;
    }
    if (!utils::is_same<char, utils::remove_reference<char>::type>())
    {
        return -2;
    }
    return 0;
}
int remove_const_check()
{
    if (!utils::is_same<char, utils::remove_const<const char>::type>())
    {
        return -1;
    }
    if (!utils::is_same<char, utils::remove_const<char>::type>())
    {
        return -2;
    }
    return 0;
}
int remove_volatile_check()
{
    if (!utils::is_same<char, utils::remove_volatile<volatile char>::type>())
    {
        return -1;
    }
    if (!utils::is_same<char, utils::remove_volatile<char>::type>())
    {
        return -2;
    }
    return 0;
}
int remove_pointer_check()
{
    if (!utils::is_same<char, utils::remove_pointer<char *>::type>())
    {
        return -1;
    }
    if (!utils::is_same<char, utils::remove_pointer<char>::type>())
    {
        return -2;
    }
    return 0;
}