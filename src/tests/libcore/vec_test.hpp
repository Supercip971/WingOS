#pragma once

#include <libcore/fmt/log.hpp>
#include <libcore/str.hpp>

#include "../test.hpp"
#include "libcore/ds/vec.hpp"
#include "libcore/mem/mem.hpp"

static constexpr TestGroup vecTests = {
    test_grouped_tests$(
        "vector",
        Test(

            "vector create",
            []() -> Test::RetFn
            {
                auto vec = core::Vec<int>();

                try$(vec.reserve(16));
                return {};
            }),
        Test(

            "vector push/pop",
            []() -> Test::RetFn
            {
                auto vec = core::Vec<int>();

                for (int i = 0; i < 16; i++)
                {
                    try$(vec.push(i));
                    if (i % 2)
                    {
                        vec.pop();
                    }
                }

                for (size_t i = 0; i < vec.len(); i++)
                {
                    if (vec[(int)i] != (int)i * 2)
                    {
                        log::log$("[{}] = {}", i, (int)vec[i]);

                        return "v[i/2] != i";
                    }
                }

                return {};
            }),
        Test(
            "vector random set",
            []() -> Test::RetFn
            {
                auto v = core::Vec<int>();

                for (int i = 0; i < 4096; i++)
                {
                    try$(v.push(i));
                }
                for (int i = 0; i < (int)v.len(); i++)
                {
                    if (v[i] != i)
                    {
                        return "v[i] != i";
                    }
                }

                v.clear();

                return {};
            }),
        Test(
            "vev.push(vec<T>())",
            []() -> Test::RetFn
            {
                auto a = core::Vec<int>();
                auto b = core::Vec<int>();

                for (int i = 0; i < 2048; i++)
                {
                    try$(a.push(i));
                    try$(b.push(i + 2048));
                }

                try$(a.push(core::move(b)));

                for (int i = 0; i < (int)a.len(); i++)
                {
                    if (a[i] != i)
                    {
                        return "a[i] != i";
                    }
                }
                if (a.len() != 4096)
                {
                    return "a.len() != 4096";
                }

                a.clear();

                return {};
            })),
};
