
#include "type_trait_check.h"
#include "unit_test.h"
#include <utils/type/integral_constant.h>
#include <utils/type/is_same.h>
#include <string.h>
#include <utils/type_traits.h>
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

LIB(type_trait)
{

    SECTION("integral constant")
    {

        CHECK("integral constant test")
        {

            utils::integral_constant<int, 0> test1;
            utils::integral_constant<int, 5> test2;
            utils::integral_constant<bool, false> test3;

            REQUIRE_EQUAL(test1.value, 0);
            REQUIRE_EQUAL(test2.value, 5);
            REQUIRE_EQUAL(test3.value, false);
            REQUIRE_EQUAL(((bool)test3), false);
            REQUIRE_EQUAL(test3(), false);
        }

        CHECK("integral constant false_type test")
        {
            REQUIRE_EQUAL(utils::false_type(), false);
        }

        CHECK("integral constant true_type test")
        {
            REQUIRE_EQUAL(utils::true_type(), true);
        }
    }

    SECTION("is_same")
    {
        CHECK("is_same positive result")
        {
            REQUIRE((utils::is_same<int, int>()));
            REQUIRE((utils::is_same<const char&, const char&>()));
            REQUIRE((utils::is_same<bool, bool>()));
        }
        CHECK("is_same negative result")
        {
            REQUIRE(!(utils::is_same<int, char>()));
            REQUIRE(!(utils::is_same<const char, const char&>()));
            REQUIRE(!(utils::is_same<bool, const bool>()));
        }
    }

    SECTION("remove type 'attribute'")
    {

        CHECK("remove_reference test")
        {
            REQUIRE((utils::is_same<char, utils::remove_reference<char &>::type>()));
            REQUIRE((utils::is_same<char, utils::remove_reference<char>::type>()));
        }
        CHECK("remove_const test")
        {
            REQUIRE((utils::is_same<char, utils::remove_const<const char>::type>()));
            REQUIRE((utils::is_same<char, utils::remove_const<char>::type>()));
        }
        CHECK("remove_volatile test")
        {
            REQUIRE((utils::is_same<char, utils::remove_volatile<volatile char>::type>()));
            REQUIRE((utils::is_same<char, utils::remove_volatile<char>::type>()));
        }

        CHECK("remove_pointer test")
        {

            REQUIRE((utils::is_same<char, utils::remove_pointer<char*>::type>()));
            REQUIRE((utils::is_same<char, utils::remove_pointer<char>::type>()));
        }

    }

    SECTION("is_class")
    {

        CHECK("is_class positive result")
        {
            REQUIRE((utils::is_class<is_type_check::a_class>()));
        }
        CHECK("is_class negative result")
        {
            REQUIRE(!(utils::is_class<is_type_check::a_enum>()));
            REQUIRE(!(utils::is_class<is_type_check::a_union>()));
        }
    }
    SECTION("is enum")
    {
        CHECK("is_enum positive result")
        {
            REQUIRE((utils::is_enum<is_type_check::a_enum>()));
        }
        CHECK("is_enum negative result")
        {
            REQUIRE(!(utils::is_enum<is_type_check::a_class>()));
            REQUIRE(!(utils::is_enum<is_type_check::a_union>()));
        }
     }

    SECTION("is_union")
    {
        CHECK("is_union positive result")
        {
            REQUIRE((utils::is_union<is_type_check::a_union>()));
        }
        CHECK("is_union negative result")
        {
            REQUIRE(!(utils::is_union<is_type_check::a_class>()));
            REQUIRE(!(utils::is_union<is_type_check::a_enum>()));
        }
    }

    SECTION("is_const")
    {

        CHECK("is_const positive result")
        {
            REQUIRE((utils::is_const<const int>()));
            REQUIRE((utils::is_const<volatile const int>()));
            REQUIRE((utils::is_const<int *const>()));
        }
        CHECK("is_const negative result")
        {
            REQUIRE(!(utils::is_const<int>()));
            REQUIRE(!(utils::is_const<int&>()));
            REQUIRE(!(utils::is_const<const int&>()));
            REQUIRE(!(utils::is_const<volatile int&>()));
            REQUIRE(!(utils::is_const<int* >()));
        }
    }
    SECTION("is_function")
    {

        CHECK("is_function positive result")
        {

            REQUIRE(utils::is_function<int(int)>());
            REQUIRE(utils::is_function<decltype(memcmp)>());
        }

        CHECK("is_function negative result")
        {
            REQUIRE(!utils::is_function<int>());
            REQUIRE(!utils::is_function<void *>());
        }
    }

    SECTION("is_base_of")
    {

        class base1 {};

        class base2 {};

        class derived1 : public base1{};

        class derived2and1 : public base2, public base1{};
        class no_derived{};

        CHECK("is_base_of positive result")
        {
            REQUIRE((utils::is_base_of<base1, base1>::value));             // base1 : base1 => true
            REQUIRE((utils::is_base_of<base1, derived1>::value));            // derived1 : base1 => true
            REQUIRE((utils::is_base_of<base1, derived2and1>::value));         // derived2and1 : base 1,  base 2 => true
            REQUIRE((utils::is_base_of<base2, derived2and1>::value));
        }
        CHECK("is_base_of negative result")
        {
            REQUIRE(!(utils::is_base_of<base1, no_derived>::value ));            // no_derived : base 1, (or) base 2 => false
            REQUIRE(!(utils::is_base_of<base2, no_derived>::value));
            REQUIRE(!(utils::is_base_of<derived1, base1>::value));            // base1 : derived => false

        }
    }
}
END_LIB(type_trait)
