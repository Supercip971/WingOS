#include "bit_check.h"
#include "unit_test.h"
#include <stdint.h>
#include <utils/bit.h>
LIB(bit)
{

    uint8_t val1 = 0b01010101;
    SECTION("get bit")
    {
        CHECK("positive return test")
        {
            for (int i = 0; i < 8; i += 2)
            {
                REQUIRE_EQUAL(utils::get_bit(val1, i), 1);
            }
        }
        CHECK("negative return test")
        {
            for (int i = 1; i < 8; i += 2)
            {
                REQUIRE_EQUAL(utils::get_bit(val1, i), 0);
            }
        }
    }

    SECTION("set bit")
    {

        for (int i = 0; i < 8; i++)
        {
            if (utils::get_bit(val1, i))
            {
                utils::set_bit(val1, i, 0);
            }
            else
            {
                utils::set_bit(val1, i, 1);
            }
        }

        CHECK("positive set test")
        {
            for (int i = 1; i < 8; i += 2)
            {
                REQUIRE_EQUAL(utils::get_bit(val1, i), 1);
            }
        }
        CHECK("negative set test")
        {
            for (int i = 0; i < 8; i += 2)
            {
                REQUIRE_EQUAL(utils::get_bit(val1, i), 0);
            }
        }
    }
}
END_LIB(bit)
