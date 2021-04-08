#include "unit_test.h"
#include <stdio.h>
#include <stdlib.h>
LIB(libc_stdlib)
{

    SECTION("abs() verification")
    {

        CHECK("abs test")
        {
            REQUIRE_EQUAL(abs(-32), 32);
            REQUIRE_EQUAL(abs(39327), 39327);
            REQUIRE_EQUAL(abs(-0), 0);
        }
    }

    SECTION("atoi() verification")
    {

        CHECK("atoi test")
        {

            REQUIRE_EQUAL(atoi("16"), 16);
            REQUIRE_EQUAL(atoi("32eeee"), 32);
            REQUIRE_EQUAL(atoi("1234567890eeee"), 1234567890);
        }
    }
}
END_LIB(libc_stdlib)

