#include "array_check.h"
#include "unit_test.h"
#include <utils/wstring.h>
LIB(wstring)
{

    SECTION("creation")
    {
        CHECK("creation test")
        {
            utils::string test = utils::string();

            REQUIRE_EQUAL(test.length(), 0);
            REQUIRE((test.c_str() != nullptr)); // by default no
        }
        CHECK("creation test from cstring")
        {
            utils::string test = utils::string("hello");

            REQUIRE_EQUAL(test.length(), 5);
            REQUIRE((test.c_str() != nullptr));
        }

    }

    SECTION("access check")
    {

        utils::string test = utils::string("01234567890w01234567890");
        CHECK("set test")
        {
            for (size_t i = 1; i < test.length(); i++)
            {
                test.get(i) = (char)i;
            }
        }
        CHECK("get test")
        {
            for (size_t i = 1; i < test.length(); i++)
            {
                REQUIRE_EQUAL(test.get(i), (char)i);
            }
        }
    }
    SECTION("equality check")
    {

        utils::string test1 = utils::string("01234567890w01234567890");
        CHECK("positive result test")
        {
            utils::string test2 = utils::string("01234567890w01234567890");
            REQUIRE(test1 == test2);
        }
        CHECK("negative result test")
        {
            utils::string test2 = utils::string("0123456789UwU1234567890");
            REQUIRE(test1 != test2);
            test2 = utils::string("owowowowow");
            REQUIRE(test1 != test2);
        }
    }
    SECTION("append check")
    {

        utils::string test1 = utils::string("today");
        CHECK("append test")
        {
            test1.append(" is monday");
            REQUIRE(test1 == "today is monday");
        }
    }
    SECTION("creation from int check")
    {

        CHECK("creation from int test")
        {

            utils::string test1 = utils::string(64);
            REQUIRE(test1 == "64");
            test1 = utils::string(999);
            REQUIRE(test1 == "999");
            test1 = utils::string(-64);
            REQUIRE(test1 == "-64");
        }
    }
}
END_LIB(wstring)
