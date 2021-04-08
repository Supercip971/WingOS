#include "alloc_array_check.h"
#include "unit_test.h"
#include <utils/alloc_array.h>

LIB(alloc_array)
{

    SECTION("alloc_array creation")
    {

        CHECK("creation test")
        {

            utils::alloc_array<int, 64> array;
            REQUIRE_EQUAL(array.size(), 64);
            REQUIRE_EQUAL(array.allocated_element_count(), 0);

            for (size_t i = 0; i < 64; i++)
            {
                REQUIRE_EQUAL(array.status(i), 0);
            }
        }
        CHECK("fill constructor test")
        {

            utils::alloc_array<int, 64> array(1);
            REQUIRE_EQUAL(array.size(), 64);
            REQUIRE_EQUAL(array.allocated_element_count(), 0);

            for (size_t i = 0; i < 64; i++)
            {
                REQUIRE_EQUAL(array.status(i), 0);
            }
        }
    }

    SECTION("alloc_array allocation & freeing")
    {
        utils::alloc_array<int, 1024> array;
        CHECK("allocation")
        {

            for (size_t i = 0; i < 512; i++)
            {
                long targ = array.alloc();
                REQUIRE(targ != -1);
                REQUIRE_EQUAL(array.status(targ), 1);
                array[targ] = targ;
            }

            REQUIRE_EQUAL(array.allocated_element_count(), 512);
        }

        CHECK("allocation id test")
        {

            size_t detected_allocated_element = 0;

            for (size_t i = 0; i < array.size(); i++)
            {
                if (array.status(i))
                {
                    detected_allocated_element++;
                    REQUIRE_EQUAL(array[i], (int)i);
                }
            }
            REQUIRE_EQUAL(array.allocated_element_count(), detected_allocated_element);
        }

        CHECK("freeing element")
        {
            for (size_t i = 0; i < array.size(); i++)
            {
                if (array.status(i))
                {
                    array.free(i);
                }
            }

            REQUIRE_EQUAL(array.allocated_element_count(), 0);
        }
    }

    MEMBER("alloc_array.foreach")
    {
        utils::alloc_array<int, 1024> array;
        CHECK("foreach allocated element test")
        {
            for (int i = 0; i < 512; i++)
            {
                array.alloc();
            }
            size_t detected_entry_count = 0;

            array.foreach_entry([&](int &value) {
                detected_entry_count++;
            });
            REQUIRE_EQUAL(detected_entry_count, 512);
        }
    }
}
END_LIB(alloc_array)
