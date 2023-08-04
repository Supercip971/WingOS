#pragma once
#include <libcore/fmt/log.hpp>

#include "libcore/fmt/impl/tabbed.hpp"
#include "libcore/result.hpp"
#include "libcore/str.hpp"

struct Test
{

    using RetFn = core::Result<void>;
    using TestFunc = RetFn (*)();
    const char *name;
    TestFunc func;
    bool expect_failure;
    const char *expected_failure_msg;

    constexpr Test() : name(nullptr), func(nullptr), expect_failure(false), expected_failure_msg(nullptr) {}

    constexpr Test(const char *name, TestFunc func, bool expect_failure, const char *expected_failure_msg) : name(name), func(func), expect_failure(expect_failure), expected_failure_msg(expected_failure_msg) {}

    template <typename T>
    constexpr Test(const char *name, T f) : name(name), func(f), expect_failure(false), expected_failure_msg(nullptr)
    {
    }
    template <typename T>
    constexpr Test(const char *name, T f, const char *expect_failure) : name(name), func(f), expect_failure(true), expected_failure_msg(expect_failure)
    {
    }
};

struct TestGroupResult
{
    size_t count;
    size_t failed;

    TestGroupResult() : count(0), failed(0) {}

    void add_result(core::Result<void> &&result)
    {
        count++;
        if (!result)
        {
            failed++;
        }
    }
};

struct TestGroup
{

    bool is_end;
    const char *name;
    size_t count;
    union
    {
        Test *tests;
        TestGroup *childs;
    };
    constexpr TestGroup() : is_end(false) {}

    constexpr TestGroup(bool is_end, const char *name, size_t count, Test *tests) : is_end(is_end), name(name), count(count), tests(tests) {}

    constexpr TestGroup(bool is_end, const char *name, size_t count, TestGroup *childs) : is_end(is_end), name(name), count(count), childs(childs) {}

    constexpr static TestGroup grouped_group(const char *name, TestGroup *childs, size_t count)
    {
        return TestGroup{false, name, count, childs};
    }

    constexpr static TestGroup grouped_tests(const char *name, Test *childs, size_t count)
    {
        return TestGroup{true, name, count, childs};
    }

    TestGroupResult execute_test(Test &tst, int depth)
    {
        TestGroupResult result;
        auto test_result = tst.func();

        // test not failed but was expected to fail
        if (test_result && tst.expect_failure)
        {
            log::err$("{} * [FAILED] Test '{}' expected failure: '{}' but got success", fmt::Tabbed(depth), tst.name, tst.expected_failure_msg);
            result.failed = 1;
        }

        // test failed but was not expected to fail
        if (!test_result && !tst.expect_failure)
        {
            log::err$("{} * [FAILED] Test '{}' with error: {}", tst.name, test_result.error());
            result.failed = 1;
        }

        // test failed and was expected to fail
        if (!test_result && tst.expect_failure)
        {
            if (core::Str(tst.expected_failure_msg) != core::Str(test_result.error()))
            {
                log::err$("{} * [FAILED] Test '{}' expected failure: '{}' but got: '{}'", fmt::Tabbed(depth), tst.name, tst.expected_failure_msg, test_result.error());
                result.failed = 1;
            }
            else
            {
                log::log$("{} * [OK] Test '{}' ", fmt::Tabbed(depth), tst.name);
                result.failed = 0;
            }
        }
        // test passed and was not expected to fail
        else if (test_result && !tst.expect_failure)
        {
            log::log$("{} * [OK] Test '{}' ", fmt::Tabbed(depth), tst.name);
            result.failed = 0;
        }
        result.count = 1;

        return result;
    }

    TestGroupResult execute(int depth)
    {
        TestGroupResult result;
        log::log$("{} -> Executing test group: {}", fmt::Tabbed(depth), name);
        if (is_end)
        {
            for (size_t i = 0; i < count; i++)
            {
                auto test_result = execute_test(tests[i], depth + 1);

                result.count += test_result.count;
                result.failed += test_result.failed;
            }

            return result;
        }
        else
        {
            for (size_t i = 0; i < count; i++)
            {

                auto child_result = childs[i].execute(depth + 1);
                result.count += child_result.count;
                result.failed += child_result.failed;
            }
        }
        if (result.failed > 0)
        {

            log::err$("{} - [FAILED] Test group {} finished with {} failed tests out of {}", fmt::Tabbed(depth), name, result.failed, result.count);
            //        log::err$("{} - [FAILED] Test group {} with {} failed tests out of {}", fmt::Tabbed(depth), childs[i].name, child_result.failed, child_result.count);
        }
        else
        {
            log::log$("{} - [OK] Test group {} finished with {} success ", fmt::Tabbed(depth), name, result.count);
        }

        return result;
    }
};

#define test_grouped_group$(name, ...) \
    TestGroup::grouped_group(name, (TestGroup[]){__VA_ARGS__}, sizeof((TestGroup[]){__VA_ARGS__}) / sizeof(TestGroup))

#define test_grouped_tests$(name, ...) \
    TestGroup::grouped_tests(name, (Test[]){__VA_ARGS__}, sizeof((Test[]){__VA_ARGS__}) / sizeof(Test))
