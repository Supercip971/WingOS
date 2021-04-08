#include "smart_ptr_check.h"
#include <utils/smart_ptr.h>
#include <utils/type_traits.h>

#include "unit_test.h"
#include <stdio.h>
LIB(smart_ptr)
{

    SECTION("unique_ptr: object creation")
    {

        CHECK("unique_ptr creation from nullptr test")
        {
            utils::unique_ptr<int> null_unique(nullptr);
            REQUIRE_EQUAL((bool)null_unique, false);
            REQUIRE_EQUAL(null_unique.get_raw(), nullptr);
        }

        CHECK("unique_ptr creation/destruction from allocated data test")
        {

            utils::unique_ptr<int> nonnull_unique(new int[2]);

            REQUIRE_EQUAL((bool)nonnull_unique, true);
            nonnull_unique.~unique_ptr();
            REQUIRE_EQUAL((bool)nonnull_unique, false);
            REQUIRE_EQUAL(nonnull_unique.get_raw(), nullptr);
        }

        CHECK("make_unique check")
        {

            auto nonnull_unique = (utils::make_unique<int>(10));
            REQUIRE_EQUAL((bool)nonnull_unique, true);
            REQUIRE_EQUAL(nonnull_unique.get(), 10);
        }
    }
    MEMBER("unique_ptr.get_raw()")
    {
        int *data = new int[10];
        utils::unique_ptr<int> nonnull_unique(data); // give ownership of data
        CHECK("get_raw() verification")
        {
            REQUIRE_EQUAL((bool)nonnull_unique, true);
            REQUIRE_EQUAL(nonnull_unique.get_raw(), data);
        }
    }

    MEMBER("unique_ptr.operator[]")
    {
        CHECK("operator[] verification")
        {
            constexpr int size_for_check = 120;
            int *data = new int[size_for_check];

            for (int i = 0; i < size_for_check; i++)
            {
                data[i] = i;
            }
            utils::unique_ptr<int> nonnull_unique(data);

            for (int i = 0; i < size_for_check; i++)
            {
                REQUIRE_EQUAL(nonnull_unique[i], i);
            }
        }
    }

    SECTION("unique_ptr: reset/release")
    {

        CHECK("reset verification")
        {
            constexpr size_t size_for_check = 120;
            int *data = new int[size_for_check];
            int *data2 = new int[size_for_check];

            data[5] = 5;
            data2[5] = 10;

            utils::unique_ptr<int> nonnull_unique(data);
            nonnull_unique.reset(data2);

            REQUIRE_EQUAL((bool)nonnull_unique, true);
            REQUIRE_EQUAL(nonnull_unique[5], data2[5]);
            REQUIRE_EQUAL(nonnull_unique.get_raw(), data2);
        }

        CHECK("release verification")
        {

            int *data = new int[10];
            utils::unique_ptr<int> nonnull_unique(data);

            int *res = nonnull_unique.release();

            REQUIRE_EQUAL((bool)nonnull_unique, false);
            REQUIRE_EQUAL(res, data);

            delete[] res;
        }
    }
}
END_LIB(smart_ptr)
