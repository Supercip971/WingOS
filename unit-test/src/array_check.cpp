#include "array_check.h"
#include "unit_test.h"
#include <utils/warray.h>

LIB(array)
{

    SECTION("array creation")
    {
        CHECK("creation test")
        {
            utils::array<int, 5> test;

            REQUIRE_EQUAL(test.size(), 5);
            REQUIRE((test.raw() != nullptr));
        }

        CHECK("fill constructor test")
        {
            utils::array<int, 5> test(16);
            for (size_t i = 0; i < test.size(); i++)
            {
                REQUIRE_EQUAL(test[i], 16);
            }
        }
    }

    SECTION("array access")
    {

        utils::array<size_t, 1024> test;
        CHECK("set test")
        {
            for (size_t i = 0; i < test.size(); i++)
            {
                test.get(i) = i;
            }
        }
        CHECK("get test")
        {
            for (size_t i = 0; i < test.size(); i++)
            {
                REQUIRE_EQUAL(test.get(i), i);
            }
        }
    }

    MEMBER("array.fill")
    {
        CHECK("fill test")
        {
            utils::array<int, 1024> test;
            test.fill(16);

            for (size_t i = 0; i < test.size(); i++)
            {
                REQUIRE_EQUAL(test.get(i), 16);
            }
        }
    }
}
END_LIB(array)
