#include "math_check.h"
#include "unit_test.h"
#include <utils/math.h>
LIB(math)
{

    SECTION("min/max")
    {

        CHECK("min result test")
        {
            REQUIRE_EQUAL(utils::min(10, 9), 9);
            REQUIRE_EQUAL(utils::min(9, 10), 9);
            REQUIRE_EQUAL(utils::min(-2, 100), -2);
            REQUIRE_EQUAL(utils::min(-10, -3), -10);
            REQUIRE_EQUAL(utils::min(10, 10), 10);
        }

        CHECK("max result test")
        {
            REQUIRE_EQUAL(utils::max(10, 9), 10);
            REQUIRE_EQUAL(utils::max(9, 10), 10);
            REQUIRE_EQUAL(utils::max(-2, 100), 100);
            REQUIRE_EQUAL(utils::max(-10, -3), -3);
            REQUIRE_EQUAL(utils::max(10, 10), 10);
        }
    }
}
END_LIB(math)
