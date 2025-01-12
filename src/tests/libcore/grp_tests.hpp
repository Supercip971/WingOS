#pragma once

#include "str_test.hpp"
#include "tests/libcore/bitmap_test.hpp"
#include "tests/libcore/llist_test.hpp"
#include "tests/libcore/vec_test.hpp"

#include "../test.hpp"

static constexpr TestGroup libcoreTests = {
    test_grouped_group$(
        "libcore tests",
        strTests,
        bitmapTests,
        vecTests,
        llistTests),
};
