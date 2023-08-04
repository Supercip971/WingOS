#pragma once

#include "../test.hpp"

static constexpr TestGroup libcoreTests = {
    test_grouped_tests$(
        "libcore tests",
        Test(

            "Always pass",
            []() -> Test::RetFn
            {
                return {};
            }),
        Test(
            "Always fail",
            []() -> Test::RetFn
            { return "Expected failure"; },
            "Expected failure")),
};
