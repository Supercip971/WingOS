#pragma once

#include "str_test.hpp"
#include "tests/libcore/bitmap_test.hpp"
#include "tests/libcore/llist_test.hpp"
#include "tests/libcore/vec_test.hpp"
#include "tests/libcore/fmt_test.hpp"
#include "tests/libcore/result_test.hpp"
#include "../test.hpp"
#include "tests/libcore/sharedptr.hpp"

static constexpr TestGroup libcoreTests = {
    test_grouped_group$(
        "libcore tests",
        strTests,
        bitmapTests,
        vecTests,
        llistTests,
        fmtTestWStr,
        sharedPtrTests,
        resultTests
    ),
};
