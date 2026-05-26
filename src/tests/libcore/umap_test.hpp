#pragma once

#include <stddef.h>
#include <libcore/fmt/log.hpp>
#include "libcore/str_writer.hpp"

struct UMapTestKey;

namespace core
{
static inline constexpr size_t hash(UMapTestKey const &key);
}

#include "libcore/ds/umap.hpp"

#include "../test.hpp"

struct UMapTestKey
{
    int value;
    static inline int construct_count = 0;
    static inline int move_count = 0;
    static inline int destruct_count = 0;

    UMapTestKey(int v = 0) : value(v)
    {
        construct_count++;
    }

    UMapTestKey(const UMapTestKey &other) : value(other.value)
    {
        construct_count++;
    }

    UMapTestKey(UMapTestKey &&other) : value(other.value)
    {
        construct_count++;
        move_count++;
    }

    UMapTestKey &operator=(const UMapTestKey &other)
    {
        value = other.value;
        return *this;
    }

    UMapTestKey &operator=(UMapTestKey &&other)
    {
        value = other.value;
        move_count++;
        return *this;
    }

    ~UMapTestKey()
    {
        destruct_count++;
    }

    bool operator==(const UMapTestKey &other) const
    {
        return value == other.value;
    }

    bool operator!=(const UMapTestKey &other) const
    {
        return value != other.value;
    }

    static int live_count()
    {
        return construct_count - destruct_count;
    }

    static void reset_counts()
    {
        construct_count = 0;
        move_count = 0;
        destruct_count = 0;
    }
};

namespace core
{
static inline constexpr size_t hash(UMapTestKey const &key)
{
    return key.value;
}
} // namespace core

struct UMapTestValue
{
    int value;
    static inline int construct_count = 0;
    static inline int move_count = 0;
    static inline int destruct_count = 0;

    UMapTestValue(int v = 0) : value(v)
    {
        construct_count++;
    }

    UMapTestValue(const UMapTestValue &other) : value(other.value)
    {
        construct_count++;
    }

    UMapTestValue(UMapTestValue &&other) : value(other.value)
    {
        construct_count++;
        move_count++;
    }

    UMapTestValue &operator=(const UMapTestValue &other)
    {
        value = other.value;
        return *this;
    }

    UMapTestValue &operator=(UMapTestValue &&other)
    {
        value = other.value;
        move_count++;
        return *this;
    }

    ~UMapTestValue()
    {
        destruct_count++;
    }

    static int live_count()
    {
        return construct_count - destruct_count;
    }

    static void reset_counts()
    {
        construct_count = 0;
        move_count = 0;
        destruct_count = 0;
    }
};

static constexpr TestGroup umapTests = {
    test_grouped_tests$(
        "umap",
        Test(
            "umap insert/has",
            []() -> Test::RetFn
            {
                auto map = core::UMap<size_t, int>();

                map.insert(1, 10);
                map.insert(2, 20);
                map.insert(3, 30);

                if (!map.has(1) || !map.has(2) || !map.has(3))
                {
                    return "missing key after insert";
                }
                if (map.has(4))
                {
                    return "unexpected key present";
                }

                return {};
            }),
        Test(
            "umap find present",
            []() -> Test::RetFn
            {
                auto map = core::UMap<size_t, int>();

                map.insert(1, 10);
                map.insert(2, 20);
                map.insert(3, 30);

                auto v1 = map.find(1);
                if (!v1.has_value())
                {
                    return "find missing key 1";
                }
                if (v1.value() != 10)
                {
                    return "find value mismatch for key 1";
                }

                auto v3 = map.find(3);
                if (!v3.has_value())
                {
                    return "find missing key 3";
                }
                if (v3.value() != 30)
                {
                    return "find value mismatch for key 3";
                }

                return {};
            }),
        Test(
            "umap find missing",
            []() -> Test::RetFn
            {
                auto map = core::UMap<size_t, int>();

                map.insert(1, 10);
                map.insert(2, 20);
                map.insert(3, 30);

                auto v4 = map.find(4);
                if (v4.has_value())
                {
                    return "find returned value for missing key";
                }

                return {};
            }),
        Test(
            "umap remove existing",
            []() -> Test::RetFn
            {
                auto map = core::UMap<size_t, int>();

                map.insert(10, 100);
                map.insert(20, 200);
                map.insert(30, 300);

                map.remove(20);

                if (map.has(20))
                {
                    return "remove failed to delete key";
                }
                if (map.find(20).has_value())
                {
                    return "find returned removed key";
                }
                if (!map.has(10) || !map.has(30))
                {
                    return "remove affected other keys";
                }

                return {};
            }),
        Test(
            "umap remove missing",
            []() -> Test::RetFn
            {
                auto map = core::UMap<size_t, int>();

                map.insert(10, 100);
                map.insert(30, 300);

                map.remove(99);

                if (!map.has(10) || !map.has(30))
                {
                    return "remove missing key altered map";
                }

                return {};
            }),
        Test(
            "basic umap",
            []() -> Test::RetFn
            {

                auto map = core::UMap<core::WStr, int>();





                map.insert(core::WStr::copy("bob"), 10);

                if(map["bob"] != 10)
                {
                    return "operator[] failed to retrieve value";
                }
                return {};
            }
            ),
        Test(
            "umap iterator",
            []() -> Test::RetFn
            {
                auto map = core::UMap<size_t, int>();

                for (size_t i = 0; i < 5; i++)
                {
                    map.insert(i, (int)(i * 10));
                }

                bool seen[5] = {false, false, false, false, false};
                size_t count = 0;

                for (auto it : map)
                {
                    auto &entry = it;
                    if (entry.key >= 5)
                    {
                        return "iterator key out of range";
                    }
                    if (seen[entry.key])
                    {
                        return "iterator returned duplicate key";
                    }
                    if (entry.value != (int)(entry.key * 10))
                    {
                        return "iterator value mismatch";
                    }
                    seen[entry.key] = true;
                    count++;
                }

                if (count != 5)
                {
                    return "iterator count mismatch";
                }

                for (size_t i = 0; i < 5; i++)
                {
                    if (!seen[i])
                    {
                        return "iterator missing key";
                    }
                }

                return {};
            }),
        Test(
            "umap key/value lifetime on insert/remove",
            []() -> Test::RetFn
            {
                UMapTestKey::reset_counts();
                UMapTestValue::reset_counts();
                {
                    auto map = core::UMap<UMapTestKey, UMapTestValue>();

                    for (int i = 0; i < 10; i++)
                    {
                        map.insert(UMapTestKey(i), UMapTestValue(i * 10));
                    }

                    if (UMapTestKey::live_count() != 10)
                    {
                        fmt::log$("key live_count: {}, expected: 10", UMapTestKey::live_count());
                        return "incorrect live key count after insert";
                    }
                    if (UMapTestValue::live_count() != 10)
                    {
                        fmt::log$("value live_count: {}, expected: 10", UMapTestValue::live_count());
                        return "incorrect live value count after insert";
                    }
                    if (UMapTestKey::move_count == 0)
                    {
                        return "keys were not moved during insert";
                    }
                    if (UMapTestValue::move_count == 0)
                    {
                        return "values were not moved during insert";
                    }

                    map.remove(UMapTestKey(1));
                    map.remove(UMapTestKey(3));
                    map.remove(UMapTestKey(5));
                    map.remove(UMapTestKey(7));

                    if (UMapTestKey::live_count() != 6)
                    {
                        fmt::log$("key live_count: {}, expected: 6", UMapTestKey::live_count());
                        return "incorrect live key count after remove";
                    }
                    if (UMapTestValue::live_count() != 6)
                    {
                        fmt::log$("value live_count: {}, expected: 6", UMapTestValue::live_count());
                        return "incorrect live value count after remove";
                    }

                    if (map.has(UMapTestKey(1)) || map.has(UMapTestKey(3)) || map.has(UMapTestKey(5)) || map.has(UMapTestKey(7)))
                    {
                        return "removed key still present";
                    }
                    if (!map.has(UMapTestKey(0)) || !map.has(UMapTestKey(2)) || !map.has(UMapTestKey(9)))
                    {
                        return "remove affected retained tracked keys";
                    }
                }

                if (UMapTestKey::construct_count != UMapTestKey::destruct_count)
                {
                    fmt::log$("key construct_count: {}, destruct_count: {}",
                              UMapTestKey::construct_count, UMapTestKey::destruct_count);
                    return "key lifetime leak after map destruction";
                }
                if (UMapTestValue::construct_count != UMapTestValue::destruct_count)
                {
                    fmt::log$("value construct_count: {}, destruct_count: {}",
                              UMapTestValue::construct_count, UMapTestValue::destruct_count);
                    return "value lifetime leak after map destruction";
                }

                return {};
            }),
        Test(
            "umap key/value lifetime on rehash/move",
            []() -> Test::RetFn
            {
                UMapTestKey::reset_counts();
                UMapTestValue::reset_counts();
                {
                    auto map = core::UMap<UMapTestKey, UMapTestValue>();

                    for (int i = 0; i < 80; i++)
                    {
                        map.insert(UMapTestKey(i), UMapTestValue(i * 100));
                    }

                    if (UMapTestKey::live_count() != 80)
                    {
                        fmt::log$("key live_count after rehash: {}, expected: 80", UMapTestKey::live_count());
                        return "incorrect live key count after rehash";
                    }
                    if (UMapTestValue::live_count() != 80)
                    {
                        fmt::log$("value live_count after rehash: {}, expected: 80", UMapTestValue::live_count());
                        return "incorrect live value count after rehash";
                    }

                    auto moved = core::UMap<UMapTestKey, UMapTestValue>(core::move(map));

                    if (UMapTestKey::live_count() != 80)
                    {
                        fmt::log$("key live_count after map move: {}, expected: 80", UMapTestKey::live_count());
                        return "incorrect live key count after map move";
                    }
                    if (UMapTestValue::live_count() != 80)
                    {
                        fmt::log$("value live_count after map move: {}, expected: 80", UMapTestValue::live_count());
                        return "incorrect live value count after map move";
                    }

                    for (int i = 0; i < 80; i++)
                    {
                        auto found = moved.find(UMapTestKey(i));
                        if (!found.has_value())
                        {
                            return "tracked key missing after rehash/move";
                        }
                        if (found.value().value != i * 100)
                        {
                            return "tracked value mismatch after rehash/move";
                        }
                    }
                }

                if (UMapTestKey::construct_count != UMapTestKey::destruct_count)
                {
                    fmt::log$("key construct_count: {}, destruct_count: {}",
                              UMapTestKey::construct_count, UMapTestKey::destruct_count);
                    return "key lifetime leak after rehash/move";
                }
                if (UMapTestValue::construct_count != UMapTestValue::destruct_count)
                {
                    fmt::log$("value construct_count: {}, destruct_count: {}",
                              UMapTestValue::construct_count, UMapTestValue::destruct_count);
                    return "value lifetime leak after rehash/move";
                }

                return {};
            })),
};
