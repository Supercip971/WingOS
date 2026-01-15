#pragma once

#include <libcore/fmt/log.hpp>
#include <libcore/str.hpp>

#include "../test.hpp"
#include "libcore/ds/vec.hpp"
#include "libcore/mem/mem.hpp"

// Helper class to track construction/destruction for vector tests
struct VecTestObject
{
    int value;
    static int construct_count;
    static int destruct_count;

    VecTestObject(int v = 0) : value(v)
    {
        construct_count++;
    }

    VecTestObject(const VecTestObject& other) : value(other.value)
    {
        construct_count++;
    }

    VecTestObject(VecTestObject&& other) : value(other.value)
    {
        construct_count++;
    }

    ~VecTestObject()
    {
        destruct_count++;
    }

    static void reset_counts()
    {
        construct_count = 0;
        destruct_count = 0;
    }
};

int VecTestObject::construct_count = 0;
int VecTestObject::destruct_count = 0;

static constexpr TestGroup vecTests = {
    test_grouped_tests$(
        "vector",
        Test(

            "vector create",
            []() -> Test::RetFn
            {
                auto vec = core::Vec<int>();

                try$(vec.reserve(16));
                return {};
            }),
        Test(

            "vector push/pop",
            []() -> Test::RetFn
            {
                auto vec = core::Vec<int>();

                for (int i = 0; i < 16; i++)
                {
                    try$(vec.push(i));
                    if (i % 2)
                    {
                        vec.pop();
                    }
                }

                for (size_t i = 0; i < vec.len(); i++)
                {
                    if (vec[(int)i] != (int)i * 2)
                    {
                        log::log$("[{}] = {}", i, (int)vec[i]);

                        return "v[i/2] != i";
                    }
                }

                return {};
            }),
        Test(
            "vector random set",
            []() -> Test::RetFn
            {
                auto v = core::Vec<int>();

                for (int i = 0; i < 4096; i++)
                {
                    try$(v.push(i));
                }
                for (int i = 0; i < (int)v.len(); i++)
                {
                    if (v[i] != i)
                    {
                        return "v[i] != i";
                    }
                }

                v.clear();

                return {};
            }),
        Test(
            "vev.push(vec<T>())",
            []() -> Test::RetFn
            {
                auto a = core::Vec<int>();
                auto b = core::Vec<int>();

                for (int i = 0; i < 2048; i++)
                {
                    try$(a.push(i));
                    try$(b.push(i + 2048));
                }

                try$(a.push(core::move(b)));

                for (int i = 0; i < (int)a.len(); i++)
                {
                    if (a[i] != i)
                    {
                        return "a[i] != i";
                    }
                }
                if (a.len() != 4096)
                {
                    return "a.len() != 4096";
                }

                a.clear();

                return {};
            }),
        Test(
            "vector constructor count on push",
            []() -> Test::RetFn
            {
                VecTestObject::reset_counts();
                {
                    auto vec = core::Vec<VecTestObject>();

                    // Push 10 elements
                    for (int i = 0; i < 10; i++)
                    {
                        try$(vec.push(VecTestObject(i)));
                    }

                    // Should have constructed at least 10 objects
                    int constructs_after_push = VecTestObject::construct_count;
                    if (constructs_after_push < 10)
                    {
                        log::log$("construct_count: {}, expected at least 10", constructs_after_push);
                        return "construct_count < 10 after push operations";
                    }

                    // Vector should have 10 elements
                    if (vec.len() != 10)
                    {
                        return "vector length != 10";
                    }
                }

                // After scope exit, all objects should be destroyed
                if (VecTestObject::construct_count != VecTestObject::destruct_count)
                {
                    log::log$("construct_count: {}, destruct_count: {}",
                              VecTestObject::construct_count, VecTestObject::destruct_count);
                    return "memory leak after scope exit";
                }

                return {};
            }),
        Test(
            "vector destructor called on clear",
            []() -> Test::RetFn
            {
                VecTestObject::reset_counts();
                {
                    auto vec = core::Vec<VecTestObject>();

                    // Push elements
                    for (int i = 0; i < 10; i++)
                    {
                        try$(vec.push(VecTestObject(i)));
                    }

                    int destructs_before_clear = VecTestObject::destruct_count;

                    // Clear the vector
                    vec.clear();

                    // All 10 objects in the vector should now be destroyed
                    if (VecTestObject::destruct_count < destructs_before_clear + 10)
                    {
                        log::log$("destruct_count: {}, expected at least: {}",
                                  VecTestObject::destruct_count, destructs_before_clear + 10);
                        return "not all objects destroyed after clear";
                    }

                    // Vector should be empty
                    if (vec.len() != 0)
                    {
                        return "vector not empty after clear";
                    }
                }

                // Verify no memory leak
                if (VecTestObject::construct_count != VecTestObject::destruct_count)
                {
                    log::log$("Final: construct_count={}, destruct_count={}",
                              VecTestObject::construct_count, VecTestObject::destruct_count);
                    return "memory leak after scope exit";
                }

                return {};
            }),
        Test(
            "vector destructor called on pop",
            []() -> Test::RetFn
            {
                VecTestObject::reset_counts();
                {
                    auto vec = core::Vec<VecTestObject>();

                    // Push 5 elements
                    for (int i = 0; i < 5; i++)
                    {
                        try$(vec.push(VecTestObject(i)));
                    }

                    int destructs_before_pop = VecTestObject::destruct_count;

                    // Pop 3 elements
                    for (int i = 0; i < 3; i++)
                    {
                        vec.pop();
                    }

                    // Should have destroyed exactly 3 more objects
                    if (VecTestObject::destruct_count < destructs_before_pop + 3)
                    {
                        log::log$("destruct_count: {}, expected at least: {}",
                                  VecTestObject::destruct_count, destructs_before_pop + 3);
                        return "destructor not called for all popped elements";
                    }

                    // Should have 2 elements remaining
                    if (vec.len() != 2)
                    {
                        return "incorrect vector length after pop";
                    }
                }

                // Verify all constructed objects are destroyed
                if (VecTestObject::construct_count != VecTestObject::destruct_count)
                {
                    log::log$("Final: construct_count={}, destruct_count={}",
                              VecTestObject::construct_count, VecTestObject::destruct_count);
                    return "memory leak: not all objects destroyed";
                }

                return {};
            })),
};
