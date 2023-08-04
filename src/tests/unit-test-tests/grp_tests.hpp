#pragma once

#include "../test.hpp"

static constexpr TestGroup unitTestTests = {
    test_grouped_tests$(
        "unit test tests",
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
