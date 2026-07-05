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
                fc::Str a = "Hello";
                fc::Str b = "Hello";
                fc::Str c = "World";

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
                fc::Str empty;

                if (empty.len() != 0)
                {
                    return "''.length() != 0";
                }

                fc::Str a = "Hello";

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
                fc::Str a = "Hello World";
                fc::Str b = a.substr(6);

                if (b != fc::Str("World"))
                {
                    return "'Hello World'.substr(6) != 'World'";
                }

                fc::Str c = a.substr(0, 5);

                if (c != fc::Str("Hello"))
                {
                    return "'Hello World'.substr(0, 5) != 'Hello'";
                }
                return {};
            })

            ),
};
