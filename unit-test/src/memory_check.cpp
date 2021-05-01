#include "memory_check.h"
#include "unit_test.h"
#include <stdio.h>
#include <utils/memory/memory.h>
#include <utils/type_traits.h>

LIB(memory)
{

    SECTION("memory object creation")
    {

        CHECK("null memory creation test")
        {
            utils::memory mem;

            REQUIRE_EQUAL((bool)mem, false);
            REQUIRE_EQUAL(mem.size(), 0);
            REQUIRE_EQUAL(mem.data(), nullptr);
        }

        CHECK("allocated memory creation test")
        {
            utils::memory mem = utils::memory::create(10);

            REQUIRE_EQUAL((bool)mem, true);
            REQUIRE_EQUAL(mem.size(), 10);
            REQUIRE(mem.data() != nullptr);
        }

        CHECK("give owner memory creation test")
        {
            constexpr int size_for_check = 120;
            char *data = new char[size_for_check];
            utils::memory mem = utils::memory::create_and_give_ownership(data, size_for_check);

            REQUIRE_EQUAL((bool)mem, true);
            REQUIRE_EQUAL(mem.size(), size_for_check);
            REQUIRE(mem.data() != nullptr);
            REQUIRE_EQUAL(mem.data(), data);
        }
    }

    SECTION("memory object destruction")
    {

        CHECK("null memory destruction test")
        {

            utils::memory mem;
            REQUIRE_EQUAL((bool)mem, false);
            REQUIRE_EQUAL(mem.data(), nullptr);
            mem.destroy();
            REQUIRE_EQUAL(mem.data(), nullptr);
        }

        CHECK("allocated memory destruction test")
        {
            utils::memory mem = utils::memory::create(10);

            mem.destroy();
            REQUIRE_EQUAL((bool)mem, false);
            REQUIRE_EQUAL(mem.size(), 0);
            mem.destroy(); // should handle multiple destroy
        }
    }

    SECTION("memory object compare")
    {

        constexpr int size_for_check = 120;
        char *data = new char[size_for_check];
        char *data2 = new char[size_for_check];

        memset(data, 64, size_for_check);
        memset(data2, 64, size_for_check);

        utils::memory mem1 = utils::memory::create_and_give_ownership(data, size_for_check);
        utils::memory mem2 = utils::memory::create_and_give_ownership(data2, size_for_check);

        CHECK("compare positive result test")
        {

            REQUIRE_EQUAL((bool)mem1.compare(mem2), true);
            REQUIRE_EQUAL((bool)(mem1 == (mem2)), true);
        }

        mem2[4] = 0; // now they are not equal
        CHECK("compare negative result test")
        {

            REQUIRE_EQUAL((bool)mem1.compare(mem2), false);
            REQUIRE_EQUAL((bool)(mem1 == (mem2)), false);
        }
    }
    MEMBER("memory.set")
    {
        constexpr int size_for_check = 120;
        utils::memory mem = utils::memory::create(size_for_check);

        CHECK("memory.set(char) test")
        {
            mem.set<char>(10);

            for (int i = 0; i < size_for_check; i++)
            {
                REQUIRE_EQUAL(mem[i], 10);
            }
        }

        utils::memory mem2 = utils::memory::create(size_for_check);
        mem2.set<char>(20);
        CHECK("memory.set(memory) test")
        {
            mem.set(mem2);

            for (int i = 0; i < size_for_check; i++)
            {
                REQUIRE_EQUAL(mem[i], 20);
            }
        }
    }

    MEMBER("memory move/copy")
    {
        CHECK("move test")
        {
            constexpr int size_for_check = 120;
            utils::memory from = utils::memory::create(size_for_check);
            from[4] = 10;
            utils::memory to = utils::move(from);
            REQUIRE_EQUAL((bool)from, false);
            REQUIRE_EQUAL(from.size(), 0);

            REQUIRE_EQUAL((bool)to, true);
            REQUIRE_EQUAL(to.size(), size_for_check);
            REQUIRE_EQUAL(to[4], 10);
        }
        CHECK("copy from memory object test")
        {
            constexpr int size_for_check = 120;

            utils::memory from = utils::memory::create(size_for_check);
            utils::memory to_memory = utils::memory::copy(from);

            REQUIRE_EQUAL((bool)from, true);
            REQUIRE_EQUAL((bool)to_memory, true);
            REQUIRE_EQUAL(from.size(), size_for_check);
            REQUIRE_EQUAL(from.size(), to_memory.size());
            REQUIRE_EQUAL(from, to_memory);
            REQUIRE(from.data() != to_memory.data());
        }
        CHECK("copy from raw data data test")
        {
            constexpr int size_for_check = 120;

            utils::memory from = utils::memory::create(size_for_check);
            utils::memory to_data = utils::memory::copy(from.data(), from.size());

            REQUIRE_EQUAL((bool)from, true);
            REQUIRE_EQUAL((bool)to_data, true);
            REQUIRE_EQUAL(from.size(), size_for_check);
            REQUIRE_EQUAL(from.size(), to_data.size());
            REQUIRE_EQUAL(from, to_data);
            REQUIRE(from.data() != to_data.data());
        }
    }
}
END_LIB(memory)
