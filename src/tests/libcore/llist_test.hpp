#pragma once
#include <libcore/fmt/log.hpp>
#include <libcore/str.hpp>

#include "libcore/ds/linked_list.hpp"

#include "../test.hpp"
#include "libcore/fmt/flags.hpp"
#include "libcore/funcs.hpp"

// Helper class to track construction/destruction for linked list tests
struct LListTestObject
{
    int value;
    static int construct_count;
    static int destruct_count;

    LListTestObject(int v = 0) : value(v)
    {
        construct_count++;
    }

    LListTestObject(const LListTestObject& other) : value(other.value)
    {
        construct_count++;
    }

    LListTestObject(LListTestObject&& other) : value(other.value)
    {
        construct_count++;
    }

    ~LListTestObject()
    {
        destruct_count++;
    }

    static void reset_counts()
    {
        construct_count = 0;
        destruct_count = 0;
    }
};

int LListTestObject::construct_count = 0;
int LListTestObject::destruct_count = 0;

static constexpr TestGroup llistTests = {
    test_grouped_tests$(
        "linked-list",
        Test(

            "linked list create",
            []() -> Test::RetFn
            {
                auto ll = core::LinkedList<int>();
                for (int i = 0; i < 10; i++)
                {
                    ll.push(10);
                }
                ll.release();

                if (ll.count() != 0)
                {
                    return "unable to release list";
                }
                return {};
            }),
        Test(

            "llist push",
            []() -> Test::RetFn
            {
                auto ll = core::LinkedList<int>();

                for (int i = 0; i < 16; i++)
                {
                    ll.push(i);
                }

                int i = 0;

                for (auto &v : ll)
                {
                    if (v != i)
                    {
                        log::log$("[{}] = {}", i, v);

                        log::log$("{} : {}", (uintptr_t)ll.begin()._ptr | fmt::FMT_HEX, (uintptr_t)ll.end()._ptr | fmt::FMT_HEX);
                        ;

                        core::forEachIdx(ll, [&](auto &val, int idx)
                                         { log::log$("[{}] = {}", idx, val); });

                        return "v[i] != i";
                    }
                    i++;
                }

                if (ll.count() != 16 || i != 16)
                {
                    log::log$("{} : {}", ll.count(), i);
                    return "count != 16";
                }

                return {};
            }),
        Test(

            "llist push/pop",
            []() -> Test::RetFn
            {
                auto ll = core::LinkedList<int>();

                for (int i = 0; i < 16; i++)
                {
                    ll.push(i);
                    if (i % 2 != 0)
                    {
                        ll.remove(ll.count() - 1);
                    }
                }
                if (ll.count() != 8)
                {
                    return "ll.count() != 8";
                }

                int i = 0;

                for (auto &v : ll)
                {
                    if (v != i * 2)
                    {
                        log::log$("[{}] = {}", i, v);

                        log::log$("{} : {}", (uintptr_t)ll.begin()._ptr | fmt::FMT_HEX, (uintptr_t)ll.end()._ptr | fmt::FMT_HEX);
                        core::forEachIdx(ll, [&](auto &val, int idx)
                                         {
                            if(idx < 100)
                            {
                                log::log$("[{}] = {}", idx, val);
                            } });

                        return "v[i/2] != i";
                    }
                    i++;
                }

                return {};
            }),

        Test(
            "llist random set",
            []() -> Test::RetFn
            {
                auto v = core::LinkedList<int>();

                for (int i = 0; i < 4096; i++)
                {
                    v.push(i);
                }
                bool has_issue = false;
                core::forEachIdx(v, [&](auto &v, int i)
                                 {
                    if(v != i)
                    {
                        has_issue = true;
                    } });

                if (has_issue)
                {
                    return "v[i] != i";
                }

                v.release();

                if (v.count() != 0)
                {
                    return "v not cleared correctly";
                }

                return {};
            }),
        Test(
            "llist constructor count on push",
            []() -> Test::RetFn
            {
                LListTestObject::reset_counts();
                {
                    auto ll = core::LinkedList<LListTestObject>();

                    // Push 10 elements
                    for (int i = 0; i < 10; i++)
                    {
                        ll.push(LListTestObject(i));
                    }

                    // Should have constructed at least 10 objects
                    int constructs_after_push = LListTestObject::construct_count;
                    if (constructs_after_push < 10)
                    {
                        log::log$("construct_count: {}, expected at least 10", constructs_after_push);
                        return "construct_count < 10 after push operations";
                    }

                    // List should have 10 elements
                    if (ll.count() != 10)
                    {
                        return "list count != 10";
                    }
                }

                // After scope exit, all objects should be destroyed
                if (LListTestObject::construct_count != LListTestObject::destruct_count)
                {
                    log::log$("construct_count: {}, destruct_count: {}",
                              LListTestObject::construct_count, LListTestObject::destruct_count);
                    return "memory leak after scope exit";
                }

                return {};
            }),
        Test(
            "llist destructor called on release",
            []() -> Test::RetFn
            {
                LListTestObject::reset_counts();
                {
                    auto ll = core::LinkedList<LListTestObject>();

                    // Push elements
                    for (int i = 0; i < 10; i++)
                    {
                        ll.push(LListTestObject(i));
                    }

                    int destructs_before_release = LListTestObject::destruct_count;

                    // Release the list
                    ll.release();

                    // All 10 objects in the list should now be destroyed
                    if (LListTestObject::destruct_count < destructs_before_release + 10)
                    {
                        log::log$("destruct_count: {}, expected at least: {}",
                                  LListTestObject::destruct_count, destructs_before_release + 10);
                        return "not all objects destroyed after release";
                    }

                    // List should be empty
                    if (ll.count() != 0)
                    {
                        return "list not empty after release";
                    }
                }

                // Verify no memory leak
                if (LListTestObject::construct_count != LListTestObject::destruct_count)
                {
                    log::log$("Final: construct_count={}, destruct_count={}",
                              LListTestObject::construct_count, LListTestObject::destruct_count);
                    return "memory leak after scope exit";
                }

                return {};
            }),
        Test(
            "llist destructor called on remove",
            []() -> Test::RetFn
            {
                LListTestObject::reset_counts();
                {
                    auto ll = core::LinkedList<LListTestObject>();

                    // Push 5 elements
                    for (int i = 0; i < 5; i++)
                    {
                        ll.push(LListTestObject(i));
                    }

                    int destructs_before_remove = LListTestObject::destruct_count;

                    // Remove 3 elements
                    for (int i = 0; i < 3; i++)
                    {
                        ll.remove(ll.count() - 1);
                    }

                    // Should have destroyed exactly 3 more objects
                    if (LListTestObject::destruct_count < destructs_before_remove + 3)
                    {
                        log::log$("destruct_count: {}, expected at least: {}",
                                  LListTestObject::destruct_count, destructs_before_remove + 3);
                        return "destructor not called for all removed elements";
                    }

                    // Should have 2 elements remaining
                    if (ll.count() != 2)
                    {
                        return "incorrect list length after remove";
                    }
                }

                // Verify all constructed objects are destroyed
                if (LListTestObject::construct_count != LListTestObject::destruct_count)
                {
                    log::log$("Final: construct_count={}, destruct_count={}",
                              LListTestObject::construct_count, LListTestObject::destruct_count);
                    return "memory leak: not all objects destroyed";
                }

                return {};
            }), ),
};
