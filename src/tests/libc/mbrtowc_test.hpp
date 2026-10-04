#pragma once

#include "../test.hpp"

// HACK: this is a dirty hack to test the mbrtowc implementation in libc/wchar.c without having to link against libc.a
// I would love to see a better solution to this, but for now this works

#define mbrtowc wos_mbrtowc
#define mbrlen wos_mbrlen
#define mbstate wos_mbstate
#define mbstate_t wos_mbstate_t

#include "libc/locale.h"
#include "libc/wchar.c"

#undef mbrtowc
#undef mbrlen
#undef mbstate
#undef mbstate_t

extern "C" int getencoding()
{
    return LC_WOS_UTF8;
}

namespace mbrtowc_test
{
static inline Test::RetFn check(const char *bytes, size_t n, size_t expected_ret, wchar_t expected_wc)
{
    wchar_t wc = 0;
    wos_mbstate_t st = {};
    size_t r = wos_mbrtowc(&wc, bytes, n, &st);
    if (r != expected_ret)
    {
        return "mbrtowc returned unexpected length";
    }
    if (expected_ret != (size_t)-1 && wc != expected_wc)
    {
        return "mbrtowc decoded unexpected code point";
    }
    return {};
}
} // namespace mbrtowc_test

// inspired by: https://github.com/managarm/mlibc/blob/81f2f9fff72d08cd04707f0d43d2982ce0b4710d/tests/ansi/utf8.c#L44
static constexpr TestGroup mbrtowcTests = {
    test_grouped_tests$(
        "mbrtowc utf-8",
        Test(
            "ascii",
            []() -> Test::RetFn
            { return mbrtowc_test::check("A", 1, 1, L'A'); }),
        Test(
            "2 bytes",
            []() -> Test::RetFn
            { return mbrtowc_test::check("\xC2\xA2", 2, 2, 0xA2); }),
        Test(
            "3 bytes",
            []() -> Test::RetFn
            { return mbrtowc_test::check("\xE0\xA4\xB9", 3, 3, 0x939); }),
        Test(
            "4 bytes",
            []() -> Test::RetFn
            { return mbrtowc_test::check("\xF0\x90\x8D\x88", 4, 4, 0x10348); }),
        Test(
            "only first character is decoded",
            []() -> Test::RetFn
            { return mbrtowc_test::check("\xC3\xA9Z", 3, 2, 0xE9); }),
        Test(
            "null pwc still returns length",
            []() -> Test::RetFn
            {
                wos_mbstate_t st = {};
                if (wos_mbrtowc(nullptr, "\xE2\x82\xAC", 3, &st) != 3)
                {
                    return "expected length 3";
                }
                return {};
            }),
        Test(
            "null s resets state",
            []() -> Test::RetFn
            {
                wos_mbstate_t st = {};
                st.state = 5;
                if (wos_mbrtowc(nullptr, nullptr, 0, &st) != 0 || st.state != 0)
                {
                    return "state not reset";
                }
                return {};
            }),
        Test(
            "invalid byte 0xFF",
            []() -> Test::RetFn
            { return mbrtowc_test::check("\xFF", 1, (size_t)-1, 0); }))};
