#include "memory_check.h"
#include <stdio.h>
#include <utils/memory.h>
#include <utils/type_traits.h>

int memory_creation_test()
{
    {
        utils::memory mem;
        if (mem)
        {
            return -1; // default memory should not be valid
        }

        // destruction test too
    }
    {
        utils::memory mem = utils::memory::create(10);
        if (!mem)
        {
            return -2; // default memory should not be valid
        }

        if (mem.size() != 10)
        {
            return -3;
        }
    }
    return 0;
}
int memory_destroy_check()
{

    utils::memory mem = utils::memory::create(10);
    utils::memory mem2;
    if (mem2)
    {
        return -1;
    }
    mem2.destroy();

    mem.destroy();
    if (mem)
    {
        return -2;
    }
    if (mem.size() != 0)
    {
        return -3;
    }

    mem.destroy(); // should handle multiple destroy

    return 0;
}
int memory_owner_check()
{
    constexpr int size_for_check = 120;
    char *data = new char[size_for_check];
    utils::memory mem = utils::memory::create_and_give_ownership(data, size_for_check);
    if (!mem)
    {
        return -1;
    }
    if (mem.data() != data)
    {
        return -2;
    }
    return 0;
}

int memory_compare_check()
{
    constexpr int size_for_check = 120;
    char *data = new char[size_for_check];
    for (int i = 0; i < size_for_check; i++)
    {
        data[i] = i;
    }
    char *data2 = new char[size_for_check];
    memcpy(data2, data, size_for_check);

    utils::memory mem1 = utils::memory::create_and_give_ownership(data, size_for_check);
    utils::memory mem2 = utils::memory::create_and_give_ownership(data2, size_for_check);

    if (!mem1.compare(mem2))
    {
        return -1;
    }
    if (mem1 != mem2)
    {
        return -2;
    }

    mem2[4] = 0; // now they are not equal

    if (mem1.compare(mem2))
    {
        return -3;
    }
    if (mem1 == mem2)
    {
        return -4;
    }
    return 0;
}
int memory_set_check()
{
    constexpr int size_for_check = 120;

    utils::memory mem1 = utils::memory::create(size_for_check);
    utils::memory mem2 = utils::memory::create(size_for_check);
    mem2.set<char>(20);
    mem1.set<char>(10); // from data

    for (int i = 0; i < size_for_check; i++)
    {
        if (mem1[i] != 10)
        {
            return i + 1;
        }
    }
    mem1.set(mem2);

    for (int i = 0; i < size_for_check; i++)
    {
        if (mem1[i] != 20)
        {
            return -(i + 1);
        }
    }
    return 0;
}
int memory_move_check()
{
    utils::memory from = utils::memory::create(10);
    from[4] = 10;
    utils::memory to = utils::move(from);

    if (from)
    {
        return -1;
    }
    if (!to)
    {
        return -2;
    }
    if (from.size() != 0)
    {
        return -3;
    }
    if (to.size() != 10)
    {
        return -4;
    }
    if (to[4] != 10)
    {
        return -5;
    }
    return 0;
}

int memory_copy_check()
{
    constexpr int size_for_check = 120;

    utils::memory from = utils::memory::create(size_for_check);
    utils::memory to_memory = utils::memory::copy(from);
    utils::memory to_data = utils::memory::copy(from.data(), from.size());

    if (!from || !to_memory || !to_data)
    {
        return -1;
    }
    if (from.size() != to_memory.size() || from.size() != to_data.size())
    {
        return -2;
    }
    if (from != to_memory || from != to_data)
    {
        return -3;
    }
    if (from.data() == to_memory.data() || from.data() == to_data.data())
    { // should be copyied and not moved
        return -4;
    }
    return 0;
}
