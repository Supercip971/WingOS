#pragma once

#include <libcore/str.hpp>

#include "../test.hpp"

static constexpr TestGroup strTests = {
    test_grouped_tests$(
        "strings",
        Test(

            "str equality",
            []() -> Test::RetFn
            {
                core::Str a = "Hello";
                core::Str b = "Hello";
                core::Str c = "World";

                if (a != b)
                {
                    return "a != b; they should be equal";
                }

                if (a == c)
                {
                    return "a == c; they should not be equals";
                }

                return {};
            }),
        Test(
            "str length",
            []() -> Test::RetFn
            {
                core::Str empty;

                if (empty.len() != 0)
                {
                    return "''.length() != 0";
                }

                core::Str a = "Hello";

                if (a.len() != 5)
                {
                    return "'Hello'.length() != 5";
                }

                return {};
            }),
        Test(
            "sub str",
            []() -> Test::RetFn
            {
                core::Str a = "Hello World";
                core::Str b = a.substr(6);

                if (b != core::Str("World"))
                {
                    return "'Hello World'.substr(6) != 'World'";
                }

                core::Str c = a.substr(0, 5);

                if (c != core::Str("Hello"))
                {
                    return "'Hello World'.substr(0, 5) != 'Hello'";
                }
                return {};
            })

            ),
};
