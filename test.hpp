#pragma once
#include <libcore/fmt/fmt.hpp>
#include <libcore/fmt/log.hpp>
#include <libcore/result.hpp>

struct Test
{
    const char *name;
    core::Result<void> (*func)();
    bool expect_failure;
    const char *expected_failure_msg;
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
    TestGroup() : is_end(false) {}

    TestGroupResult execute(int depth)
    {
        TestGroupResult result;
        log::log$("Executing test group: {}", name);
        if (is_end)
        {
            for (size_t i = 0; i < count; i++)
            {
                auto test_result = tests[i].func();
                if (!test_result)
                {
                    log::err$(" * [FAILED] Test {} with error: {}", tests[i].name, test_result.error());
                }

                log::log$(" * [OK] Test {} ", tests[i].name);

                result.add_result(core::move(test_result));
            }

            log::log$("Test group {} finished with {} failed tests out of {}", name, result.failed, result.count);
            return result;
        }

        for (size_t i = 0; i < count; i++)
        {

            auto child_result = childs[i].execute(depth + 1);
            result.count += child_result.count;
            result.failed += child_result.failed;

            if (child_result.failed > 0)
            {
                log::err$(" - [FAILED] Test group {} with {} failed tests out of {}", childs[i].name, child_result.failed, child_result.count);
            }
            else
            {
                log::log$(" - [OK] Test group {} finished with {} failed tests out of {}", childs[i].name, child_result.failed, child_result.count);
            }
        }

        log::log$("Test group {} finished with {} failed tests out of {}", name, result.failed, result.count);

        return result;
    }
};
