#pragma once

#include <libcore/shared.hpp>
#include <libcore/fmt/log.hpp>

#include "../test.hpp"

// Helper class to track construction/destruction
struct TestObject
{
    int value;
    static int construct_count;
    static int destruct_count;

    TestObject(int v = 0) : value(v)
    {
        construct_count++;
    }

    ~TestObject()
    {
        destruct_count++;
    }

    static void reset_counts()
    {
        construct_count = 0;
        destruct_count = 0;
    }
};

int TestObject::construct_count = 0;
int TestObject::destruct_count = 0;

static constexpr TestGroup sharedPtrTests = {
    test_grouped_tests$(
        "shared_ptr",
        Test(
            "shared_ptr construction",
            []() -> Test::RetFn
            {
                TestObject::reset_counts();
                {
                    auto ptr = core::SharedPtr<TestObject>::make(42);

                    if (TestObject::construct_count != 1)
                    {
                        return "construct_count != 1 after construction";
                    }
                }

                if (TestObject::destruct_count != 1)
                {
                    return "destruct_count != 1 after scope exit";
                }

                return {};
            }),

        Test(
            "shared_ptr dereference operators",
            []() -> Test::RetFn
            {
                auto ptr = core::SharedPtr<TestObject>::make(100);

                if ((*ptr).value != 100)
                {
                    return "operator* failed";
                }

                if (ptr->value != 100)
                {
                    return "operator-> failed";
                }

                ptr->value = 200;

                if ((*ptr).value != 200)
                {
                    return "value not modified through operator->";
                }

                return {};
            }),

        Test(
            "shared_ptr copy semantics",
            []() -> Test::RetFn
            {
                TestObject::reset_counts();
                {
                    auto ptr1 = core::SharedPtr<TestObject>::make(50);

                    if (TestObject::construct_count != 1)
                    {
                        return "construct_count != 1 after first ptr";
                    }

                    {
                        auto ptr2 = ptr1;  // Copy constructor

                        if (TestObject::construct_count != 1)
                        {
                            return "construct_count != 1 after copy (should share)";
                        }

                        if (ptr2->value != 50)
                        {
                            return "ptr2 value incorrect";
                        }

                        ptr2->value = 75;

                        if (ptr1->value != 75)
                        {
                            return "ptr1 not sharing data with ptr2";
                        }
                    }

                    // ptr2 destroyed, but ptr1 still alive
                    if (TestObject::destruct_count != 0)
                    {
                        return "object destroyed while ptr1 still exists";
                    }

                    if (ptr1->value != 75)
                    {
                        return "ptr1 value changed after ptr2 destruction";
                    }
                }

                // Both pointers destroyed
                if (TestObject::destruct_count != 1)
                {
                    return "destruct_count != 1 after all ptrs destroyed";
                }

                return {};
            }),

        Test(
            "shared_ptr copy assignment",
            []() -> Test::RetFn
            {
                TestObject::reset_counts();
                {
                    auto ptr1 = core::SharedPtr<TestObject>::make(10);
                    auto ptr2 = core::SharedPtr<TestObject>::make(20);

                    if (TestObject::construct_count != 2)
                    {
                        return "construct_count != 2";
                    }

                    ptr2 = ptr1;  // Copy assignment

                    // ptr2's old object should be destroyed
                    if (TestObject::destruct_count != 1)
                    {
                        return "old ptr2 object not destroyed";
                    }

                    if (ptr2->value != 10)
                    {
                        return "ptr2 not pointing to ptr1's data";
                    }

                    ptr1->value = 30;

                    if (ptr2->value != 30)
                    {
                        return "ptr2 not sharing with ptr1";
                    }
                }

                if (TestObject::destruct_count != 2)
                {
                    return "destruct_count != 2 at end";
                }

                return {};
            }),

        Test(
            "shared_ptr move semantics",
            []() -> Test::RetFn
            {
                TestObject::reset_counts();
                {
                    auto ptr1 = core::SharedPtr<TestObject>::make(99);

                    auto ptr2 = core::move(ptr1);  // Move constructor

                    if (TestObject::construct_count != 1)
                    {
                        return "construct_count != 1 after move";
                    }

                    if (ptr2->value != 99)
                    {
                        return "ptr2 value incorrect after move";
                    }
                }

                if (TestObject::destruct_count != 1)
                {
                    return "destruct_count != 1 after move";
                }

                return {};
            }),

        Test(
            "shared_ptr move assignment",
            []() -> Test::RetFn
            {
                TestObject::reset_counts();
                {
                    auto ptr1 = core::SharedPtr<TestObject>::make(111);
                    auto ptr2 = core::SharedPtr<TestObject>::make(222);

                    ptr2 = core::move(ptr1);  // Move assignment

                    // ptr2's old object should be destroyed
                    if (TestObject::destruct_count != 1)
                    {
                        return "old ptr2 object not destroyed on move assignment";
                    }

                    if (ptr2->value != 111)
                    {
                        return "ptr2 value incorrect after move assignment";
                    }
                }

                if (TestObject::destruct_count != 2)
                {
                    return "destruct_count != 2 at end";
                }

                return {};
            }),

        Test(
            "shared_ptr multiple copies",
            []() -> Test::RetFn
            {
                TestObject::reset_counts();
                {
                    auto ptr1 = core::SharedPtr<TestObject>::make(5);
                    auto ptr2 = ptr1;
                    auto ptr3 = ptr2;
                    auto ptr4 = ptr3;

                    if (TestObject::construct_count != 1)
                    {
                        return "construct_count != 1 with 4 copies";
                    }

                    if (ptr4->value != 5)
                    {
                        return "ptr4 value incorrect";
                    }

                    ptr4->value = 15;

                    if (ptr1->value != 15 || ptr2->value != 15 || ptr3->value != 15)
                    {
                        return "all pointers not sharing same data";
                    }

                    if (TestObject::destruct_count != 0)
                    {
                        return "object destroyed prematurely";
                    }
                }

                if (TestObject::destruct_count != 1)
                {
                    return "destruct_count != 1 after all copies destroyed";
                }

                return {};
            }),

        Test(
            "shared_ptr self assignment",
            []() -> Test::RetFn
            {
                TestObject::reset_counts();
                {
                    auto ptr = core::SharedPtr<TestObject>::make(123);
                    ptr = ptr;  // Self assignment

                    if (TestObject::construct_count != 1)
                    {
                        return "construct_count changed after self assignment";
                    }

                    if (TestObject::destruct_count != 0)
                    {
                        return "object destroyed on self assignment";
                    }

                    if (ptr->value != 123)
                    {
                        return "value changed after self assignment";
                    }
                }

                if (TestObject::destruct_count != 1)
                {
                    return "destruct_count != 1 at end";
                }

                return {};
            }),

        Test(
            "shared_ptr with simple types",
            []() -> Test::RetFn
            {
                auto int_ptr = core::SharedPtr<int>::make(42);

                if (*int_ptr != 42)
                {
                    return "int SharedPtr value incorrect";
                }

                *int_ptr = 100;

                if (*int_ptr != 100)
                {
                    return "int SharedPtr modification failed";
                }

                auto copy = int_ptr;
                *copy = 200;

                if (*int_ptr != 200)
                {
                    return "int SharedPtr copies not sharing";
                }

                return {};
            })
    ),
};
