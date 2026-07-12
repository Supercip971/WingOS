#pragma once

#include <libcore/fmt/log.hpp>

#include "../test.hpp"
#include "libcore/ds/ring.hpp"

// Helper class to track construction/destruction for ring tests
struct RingTestObject
{
    int value;
    inline static int construct_count = 0;
    inline static int destruct_count = 0;

    RingTestObject(int v = 0) : value(v)
    {
        construct_count++;
    }

    RingTestObject(const RingTestObject &other) : value(other.value)
    {
        construct_count++;
    }

    RingTestObject(RingTestObject &&other) : value(other.value)
    {
        construct_count++;
    }

    ~RingTestObject()
    {
        destruct_count++;
    }

    static void reset_counts()
    {
        construct_count = 0;
        destruct_count = 0;
    }

    auto operator=(const RingTestObject &other)
    {
        value = other.value;
        return *this;
    }

    auto operator=(RingTestObject &&other)
    {
        value = other.value;
        return *this;
    }
};

static constexpr TestGroup ringTests = {
    test_grouped_tests$(
        "ring",
        Test(
            "ring create",
            []() -> Test::RetFn
            {
                auto ring = fc::Ring<int>();

                if (ring.len() != 0)
                {
                    return "ring.len() != 0";
                }

                return {};
            }),
        Test(
            "ring push/pop order",
            []() -> Test::RetFn
            {
                auto ring = fc::Ring<int>();

                for (int i = 0; i < 16; i++)
                {
                    ring.push(i);
                }

                if (ring.len() != 16)
                {
                    return "ring.len() != 16";
                }

                for (int i = 0; i < 16; i++)
                {
                    int value = ring.pop();
                    if (value != i)
                    {
                        fmt::log$("expected {}, got {}", i, value);
                        return "ring pop order broken";
                    }
                }

                if (ring.len() != 0)
                {
                    return "ring.len() != 0 after drain";
                }

                return {};
            }),
        Test(
            "ring wraparound order",
            []() -> Test::RetFn
            {
                auto ring = fc::Ring<int>();

                for (int i = 0; i < 8; i++)
                {
                    ring.push(i);
                }

                for (int i = 0; i < 3; i++)
                {
                    int value = ring.pop();
                    if (value != i)
                    {
                        fmt::log$("expected {}, got {}", i, value);
                        return "ring pop order broken before wraparound";
                    }
                }

                for (int i = 8; i < 14; i++)
                {
                    ring.push(i);
                }

                const int expected[] = {3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13};
                for (int i = 0; i < 11; i++)
                {
                    int value = ring.pop();
                    if (value != expected[i])
                    {
                        fmt::log$("expected {}, got {}", expected[i], value);
                        return "ring wraparound order broken";
                    }
                }

                if (ring.len() != 0)
                {
                    return "ring.len() != 0 after wraparound drain";
                }

                return {};
            }),
        Test(
            "ring destructor called on pop",
            []() -> Test::RetFn
            {
                RingTestObject::reset_counts();
                {
                    auto ring = fc::Ring<RingTestObject>();

                    for (int i = 0; i < 10; i++)
                    {
                        ring.push(RingTestObject(i));
                    }

                    for (int i = 0; i < 4; i++)
                    {
                        auto popped = ring.pop();
                        if (popped.value != i)
                        {
                            return "ring pop returned wrong value";
                        }
                    }

                    if (ring.len() != 6)
                    {
                        return "ring.len() != 6 after pop";
                    }

                    while (ring.len() != 0)
                    {
                        ring.pop();
                    }
                }

                if (RingTestObject::construct_count != RingTestObject::destruct_count)
                {
                    fmt::log$("construct_count: {}, destruct_count: {}",
                              RingTestObject::construct_count, RingTestObject::destruct_count);
                    return "memory leak after scope exit";
                }

                return {};
            }),
        Test(
            "ring copy constructor",
            []() -> Test::RetFn
            {
                auto ring = fc::Ring<int>();
                for (int i = 0; i < 5; i++)
                {
                    ring.push(i);
                }

                auto ring2 = fc::Ring<int>(ring);

                if (ring2.len() != 5)
                {
                    return "ring copy constructor length mismatch";
                }

                for (int i = 0; i < 5; i++)
                {
                    int value = ring2.pop();
                    if (value != i)
                    {
                        fmt::log$("expected {}, got {}", i, value);
                        return "ring copy constructor order broken";
                    }
                }

                for (int i = 0; i < 5; i++)
                {
                    int value = ring.pop();
                    if (value != i)
                    {
                        fmt::log$("expected {}, got {}", i, value);
                        return "ring copy constructor affected original";
                    }
                }

                return {};
            }),
        Test(
            "ring move constructor",
            []() -> Test::RetFn
            {
                auto ring = fc::Ring<int>();
                for (int i = 0; i < 5; i++)
                {
                    ring.push(i);
                }

                auto ring2 = fc::Ring<int>(fc::move(ring));

                if (ring.len() != 0)
                {
                    return "ring move constructor did not empty source";
                }

                if (ring2.len() != 5)
                {
                    return "ring move constructor length mismatch";
                }

                for (int i = 0; i < 5; i++)
                {
                    int value = ring2.pop();
                    if (value != i)
                    {
                        fmt::log$("expected {}, got {}", i, value);
                        return "ring move constructor order broken";
                    }
                }

                return {};
            }),
        Test(
            "ring single element",
            []() -> Test::RetFn
            {
                auto ring = fc::Ring<int>();

                ring.push(42);

                if (ring.len() != 1)
                {
                    return "ring single element length != 1";
                }

                int value = ring.pop();
                if (value != 42)
                {
                    fmt::log$("expected 42, got {}", value);
                    return "ring single element value mismatch";
                }

                if (ring.len() != 0)
                {
                    return "ring single element length != 0 after pop";
                }

                return {};
            }),
        Test(
            "ring resize preserves order",
            []() -> Test::RetFn
            {
                auto ring = fc::Ring<int>();

                for (int i = 0; i < 8; i++)
                {
                    ring.push(i);
                }

                ring.resize(32);

                if (ring.len() != 8)
                {
                    return "ring resize changed length";
                }

                for (int i = 0; i < 8; i++)
                {
                    int value = ring.pop();
                    if (value != i)
                    {
                        fmt::log$("expected {}, got {}", i, value);
                        return "ring resize order broken";
                    }
                }

                return {};
            }),
        Test(
            "ring resize with wraparound preserves order",
            []() -> Test::RetFn
            {
                auto ring = fc::Ring<int>();

                for (int i = 0; i < 8; i++)
                {
                    ring.push(i);
                }

                for (int i = 0; i < 4; i++)
                {
                    ring.pop();
                }

                for (int i = 8; i < 12; i++)
                {
                    ring.push(i);
                }

                ring.resize(32);

                const int expected[] = {4, 5, 6, 7, 8, 9, 10, 11};
                for (int i = 0; i < 8; i++)
                {
                    int value = ring.pop();
                    if (value != expected[i])
                    {
                        fmt::log$("expected {}, got {}", expected[i], value);
                        return "ring resize wraparound order broken";
                    }
                }

                return {};
            }),
        Test(
            "ring destructor called on scope exit",
            []() -> Test::RetFn
            {
                RingTestObject::reset_counts();

                {
                    auto ring = fc::Ring<RingTestObject>();
                    for (int i = 0; i < 10; i++)
                    {
                        ring.push(RingTestObject(i));
                    }
                }

                if (RingTestObject::construct_count != RingTestObject::destruct_count)
                {
                    fmt::log$("construct_count: {}, destruct_count: {}",
                              RingTestObject::construct_count, RingTestObject::destruct_count);
                    return "ring destructor not called on scope exit";
                }

                return {};
            }),
        Test(
            "ring stress test many elements",
            []() -> Test::RetFn
            {
                auto ring = fc::Ring<int>();

                for (int i = 0; i < 10000; i++)
                {
                    ring.push(i);
                }

                if (ring.len() != 10000)
                {
                    fmt::log$("expected length 10000, got {}", ring.len());
                    return "ring stress test length mismatch";
                }

                for (int i = 0; i < 10000; i++)
                {
                    int value = ring.pop();
                    if (value != i)
                    {
                        fmt::log$("expected {}, got {}", i, value);
                        return "ring stress test order broken";
                    }
                }

                if (ring.len() != 0)
                {
                    return "ring stress test length != 0 after drain";
                }

                return {};
            }),
        Test(
            "ring push const ref",
            []() -> Test::RetFn
            {
                auto ring = fc::Ring<int>();

                int a = 1;
                int b = 2;
                int c = 3;

                ring.push(a);
                ring.push(b);
                ring.push(c);

                if (ring.len() != 3)
                {
                    return "ring push const ref length mismatch";
                }

                if (ring.pop() != 1)
                {
                    return "ring push const ref value a mismatch";
                }
                if (ring.pop() != 2)
                {
                    return "ring push const ref value b mismatch";
                }
                if (ring.pop() != 3)
                {
                    return "ring push const ref value c mismatch";
                }

                return {};
            }),
        Test(
            "ring move semantics with RingTestObject",
            []() -> Test::RetFn
            {
                RingTestObject::reset_counts();

                {
                    auto ring = fc::Ring<RingTestObject>();

                    for (int i = 0; i < 10; i++)
                    {
                        ring.push(RingTestObject(i));
                    }

                    auto ring2 = fc::Ring<RingTestObject>(fc::move(ring));

                    if (ring.len() != 0)
                    {
                        return "ring move constructor did not empty source RingTestObject";
                    }

                    while (ring2.len() > 0)
                    {
                        ring2.pop();
                    }
                }

                if (RingTestObject::construct_count != RingTestObject::destruct_count)
                {
                    fmt::log$("construct_count: {}, destruct_count: {}",
                              RingTestObject::construct_count, RingTestObject::destruct_count);
                    return "ring move semantics memory leak";
                }

                return {};
            })),
};
