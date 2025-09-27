#pragma once
#include "../test.hpp"
#include "libcore/mem/view.hpp"
#include "parser/scanner.hpp"

static constexpr TestGroup parserTest = {
    test_grouped_tests$(
        "scanner test",
        Test(

            "Basic cursor verification",
            []() -> Test::RetFn
            {
                core::Scanner<char> scanner(core::MemView<char>(core::Str("Hello, World!")));
                if (scanner.size() != 13)
                {
                    return "Size mismatch";
                }
                if (scanner.tell() != 0)
                {
                    return "Tell mismatch";
                }
                if (scanner.ended())
                {
                    return "Scanner ended prematurely";
                }
                if (scanner.remaining() != 13)
                {
                    return "Remaining size mismatch";
                }
                auto res = scanner.current();
                if (!res)
                {
                    return res.error();
                }
                if (res.unwrap() != 'H')
                {
                    return "Current character mismatch";
                }

                scanner.seek(7);
                if (scanner.tell() != 7)
                {
                    return "Seek failed";
                }
                res = scanner.current();
                if (!res)
                {
                    return res.error();
                }
                if (res.unwrap() != 'W')
                {
                    return "Current character mismatch after seek";
                }

                return {};
            }),
        Test(
            "Parsing skipping",
            []() -> Test::RetFn
            {
                core::Scanner<char> scanner(core::MemView<char>(core::Str("Hello, World!")));
                if (!scanner.skip('H'))
                {
                    return "Failed to skip 'H'";
                }

                if (scanner.tell() != 1)
                {
                    return "Tell mismatch after skip";
                }

                if (!scanner.skip_string(core::MemView<char>(core::Str("ello, "))))
                {
                    return "Failed to skip 'ello, '";
                }
                if (scanner.tell() != 7)
                {
                    return "Tell mismatch after skip_string";
                }

                if (!scanner.skip_any_of(core::MemView<char>(core::Str("W!"))))
                {
                    return "Failed to skip 'W' or '!'";
                }
                if (scanner.tell() != 8)
                {
                    return "Tell mismatch after skip_any_of";
                }
                if (scanner.ended())
                {
                    return "Scanner ended prematurely after skips";
                }
                if (scanner.remaining() != 13 - 8)
                {
                    return "Remaining size mismatch after skips";
                }

                return {};
            }),
        Test(
            "Parsing extract",
            []() -> Test::RetFn
            {
                core::Scanner<char> scanner(core::MemView<char>(core::Str("Hello, World!")));
                auto res = scanner.read_until(',');
                if (!res)
                {
                    return res.error();
                }
                if (res.unwrap() != core::Str("Hello"))
                {
                    return "Extracted string mismatch";
                }
                if (scanner.tell() != 5)
                {
                    return "Tell mismatch after extract";
                }
                return {};
            }))};
