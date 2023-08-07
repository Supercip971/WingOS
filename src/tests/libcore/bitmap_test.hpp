#pragma once

#include <libcore/fmt/log.hpp>
#include <libcore/str.hpp>

#include "../test.hpp"
#include "libcore/ds/bitmap.hpp"
#include "libcore/mem/mem.hpp"

static uint8_t tbuf[128];

static constexpr TestGroup bitmapTests = {
    test_grouped_tests$(
        "bitmap",
        Test(

            "bitmap fill",
            []() -> Test::RetFn
            {
                auto v = core::Bitmap(core::MemAccess<>(tbuf, sizeof(tbuf)));

                v.fill(true);
                for (size_t i = 0; i < v.len(); i++)
                {
                    if (!v[i])
                    {
                        return "v[i] != true";
                    }
                }

                v.fill(false);

                for (size_t i = 0; i < v.len(); i++)
                {
                    if (v[i])
                    {
                        return "v[i] != false";
                    }
                }
                return {};
            }),
        Test(
            "bitmap random set",
            []() -> Test::RetFn
            {
                auto v = core::Bitmap(core::MemAccess<>(tbuf, sizeof(tbuf)));

                v.fill(false);

                for (size_t i = 0; i < v.len(); i++)
                {
                    v.bit(i, i % 2);
                }
                for (size_t i = 0; i < v.len(); i++)
                {
                    if (v[i] != (i % 2))
                    {
                        return "v[i] != i % 2";
                    }
                }

                return {};
            })),
};
