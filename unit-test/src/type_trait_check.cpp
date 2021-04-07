
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

namespace is_type_check
{
    union a_union
    {
        int v;
        int v2;
    };

    class a_class
    {
    public:
        int va;
        int vb;
    };

    enum a_enum
    {
        ENUM1,
        ENUM2
    };
} // namespace is_type_check

int is_class_check()
{
    if (!utils::is_class<is_type_check::a_class>())
    {
        return -1;
    }
    if (utils::is_class<is_type_check::a_enum>())
    {
        return -2;
    }
    if (utils::is_class<is_type_check::a_union>())
    {
        return -3;
    }
    return 0;
}

int is_enum_check()
{

    if (utils::is_enum<is_type_check::a_class>())
    {
        return -1;
    }
    if (!utils::is_enum<is_type_check::a_enum>())
    {
        return -2;
    }
    if (utils::is_enum<is_type_check::a_union>())
    {
        return -3;
    }
    return 0;
}

int is_union_check()
{

    if (utils::is_union<is_type_check::a_class>())
    {
        return -1;
    }
    if (utils::is_union<is_type_check::a_enum>())
    {
        return -2;
    }
    if (!utils::is_union<is_type_check::a_union>())
    {
        return -3;
    }
    return 0;
}

int is_base_of_check()
{
    class base1
    {
    public:
        int v;
    };

    class base2
    {
    public:
        int v2;
    };

    class derived1 : public base1
    {
    };

    class derived2and1 : public base2, public base1
    {
    };

    class no_derived
    {
    };

    // base1 : base1 => true
    if (!utils::is_base_of<base1, base1>::value)
    {
        return 1;
    }

    // base1 : base2 => false
    if (utils::is_base_of<base1, base2>::value)
    {
        return 2;
    }
    // derived1 : base1 => true
    if (!utils::is_base_of<base1, derived1>::value)
    {
        return 3;
    }
    // base1 : derived => false
    if (utils::is_base_of<derived1, base1>::value)
    {
        return 4;
    }
    // derived2and1 : base 1,  base 2 => true
    if (!(utils::is_base_of<base1, derived2and1>::value && utils::is_base_of<base2, derived2and1>::value))
    {
        return 5;
    }
    // no_derived : base 1, (or) base 2 => false
    if ((utils::is_base_of<base1, no_derived>::value || utils::is_base_of<base2, no_derived>::value))
    {
        return 6;
    }
    // derived1* : base1 => false
    if (utils::is_base_of<base1, derived1 *>::value)
    {
        return 7;
    }
    return 0;
}


int is_const_check(){
    // positive result check
    if(!utils::is_const<const int>()){
        return -1;
    }
    if(!utils::is_const<volatile const int>()){
        return -2;
    }
    if(!utils::is_const<int* const>()){
        return -3;
    }

    // negative result check

    if(utils::is_const<int>()){
        return -5;
    }
    if(utils::is_const<int&>()){
        return -6;
    }
    if(utils::is_const<const int&>()){
        return -7;
    }
    if(utils::is_const<volatile int&>()){
        return -8;
    }
    if(utils::is_const<int*>()){
        return -9;
    }

    return 0;


}
int is_function_check(){
    // positive result check
    if(!utils::is_function<int(int)>()){
        return -1;
    }
    if(!utils::is_function<decltype (is_function_check)>()){
        return -3;
    }
    // negative result check
    if(utils::is_function<int>()){
        return -4;
    }
    if(utils::is_function<void*>()){
        return -5;
    }
    if(utils::is_function<bool>()){
        return -6;
    }
    return 0;
}
