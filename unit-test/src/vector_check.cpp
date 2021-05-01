#include "vector_check.h"
#include "unit_test.h"
#include <stdio.h>

#include <utils/container/wvector.h>

LIB(wvector)
{

    SECTION("vector creation")
    {

        CHECK("vector creation test")
        {
            utils::vector<uint8_t> vec = utils::vector<uint8_t>();
            REQUIRE_EQUAL(vec.raw(), nullptr);
            REQUIRE_EQUAL(vec.size(), 0);
            REQUIRE_EQUAL(vec.capacity(), 0);
            REQUIRE_EQUAL(vec.size(), 0);
        }
    }

    MEMBER("vector.reserve/capacity")
    {

        CHECK("vector reserve test")
        {

            utils::vector<uint8_t> vec = utils::vector<uint8_t>();
            vec.reserve(10);

            REQUIRE(vec.raw() != nullptr);
            REQUIRE(vec.capacity() != 0);
            REQUIRE_EQUAL(vec.size(), 0);
        }

        CHECK("vector capacity test")
        {

            utils::vector<uint8_t> vec = utils::vector<uint8_t>();
            vec.reserve(10);

            REQUIRE_EQUAL(vec.capacity(), 10);
            vec.reserve(7);
            REQUIRE_EQUAL(vec.capacity(), 10);
            vec.reserve(30);
            REQUIRE_EQUAL(vec.capacity(), 30);
            REQUIRE_EQUAL(vec.size(), 0);
        }
    }

    MEMBER("vector.push_back")
    {

        utils::vector<uint8_t> vec = utils::vector<uint8_t>();
        CHECK("vector push_back() test")
        {
            for (int i = 0; i < 2048; i++)
            {
                vec.push_back(i);
            }
            REQUIRE_EQUAL(vec.size(), 2048);
        }
    }
    MEMBER("vector.get")
    {
        constexpr int size_to_check = 2048;
        utils::vector<int> vec = utils::vector<int>();

        CHECK("vector get() test")
        {
            for (int i = 0; i < size_to_check; i++)
            {
                vec.push_back(i);
            }

            for (int i = 0; i < size_to_check; i++)
            {
                REQUIRE_EQUAL(vec.get(i), vec[i]);
                REQUIRE_EQUAL(i, vec[i]);
            }
        }
    }
    MEMBER("vector.remove")
    {

        constexpr int size_to_check = 2048;
        utils::vector<int> vec = utils::vector<int>();

        CHECK("vector remove() test")
        {

            for (int i = 0; i < size_to_check; i++)
            {
                vec.push_back(i);
            }

            int off = 0;
            for (int i = 0; i < size_to_check; i++)
            {
                if ((i % 2) != 0)
                {
                    vec.remove(i + off); // use offset because every time we delete an entry everything is moved by one entry
                    off -= 1;
                }
            }
            REQUIRE_EQUAL(vec.size(), size_to_check / 2);
            for (int i = 0; i < size_to_check / 2; i++)
            {
                REQUIRE_EQUAL(i * 2, vec.get(i));
            }
        }
    }

    MEMBER("vector.clear")
    {

        constexpr int size_to_check = 2048;
        CHECK("vector clear() test")
        {

            utils::vector<int> vec = utils::vector<int>();
            for (int i = 0; i < size_to_check; i++)
            {
                vec.push_back(i);
            }
            vec.clear();
            REQUIRE(!(bool)vec);
            REQUIRE_EQUAL(vec.size(), 0);
            REQUIRE_EQUAL(vec.capacity(), 0);
        }
    }
}
END_LIB(wvector)
