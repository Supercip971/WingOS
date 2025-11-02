#pragma once

#include <libcore/str.hpp>
#include <libcore/str_writer.hpp>
#include <libcore/fmt/fmt_str.hpp>

#include "../test.hpp"


// test ultime pour voir si une IA est capable d'écrire des tests
static constexpr TestGroup fmtTestWStr = {
     test_grouped_tests$(
        "fmt + wstr",
        Test(

            "linked list create",
            []() -> Test::RetFn
            {
                
                return {};
            }),
        Test(
            "WStr write",
            []() -> Test::RetFn
            {
                core::WStr writer;
                writer.write("hello", 5);
                if (writer.view() != core::Str("hello")) {
                    return "WStr write failed";
                }
                return {};
            }),
        Test(
            "WStr put/append",
            []() -> Test::RetFn
            {
                core::WStr writer = core::WStr::copy(core::Str("ab"));

                writer.put('c');
                if (writer.view() != core::Str("abc")) {
                    return "WStr put failed";
                }
                writer.append(core::Str("def"));
                if (writer.view() != core::Str("abcdef")) {
                    return "WStr append failed";
                }
                return {};
            }),
        Test(
            "format_str basic",
            []() -> Test::RetFn
            {
                auto r = fmt::format_str("Value: {} - {}", 42, "test");
                if (r.is_error()) {
                    return "format_str returned error";
                }
                auto writer = r.take();
                if (writer.view() != core::Str("Value: 42 - test")) {
                    return "format_str result mismatch";
                }
                return {};
            }),
    )
};
