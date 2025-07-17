#pragma once
#include <libcore/fmt/log.hpp>
#include <libcore/str.hpp>

#include "libcore/ds/linked_list.hpp"

#include "../test.hpp"
#include "libcore/fmt/flags.hpp"
#include "libcore/funcs.hpp"

static constexpr TestGroup llistTests = {
    test_grouped_tests$(
        "linked-list",
        Test(

            "linked list create",
            []() -> Test::RetFn
            {
                auto ll = core::LinkedList<int>();
                for (int i = 0; i < 10; i++)
                {
                    ll.push(10);
                }
                ll.release();

                if (ll.count() != 0)
                {
                    return "unable to release list";
                }
                return {};
            }),
        Test(

            "llist push",
            []() -> Test::RetFn
            {
                auto ll = core::LinkedList<int>();

                for (int i = 0; i < 16; i++)
                {
                    ll.push(i);
                }

                int i = 0;

                for (auto &v : ll)
                {
                    if (v != i)
                    {
                        log::log$("[{}] = {}", i, v);

                        log::log$("{} : {}", (uintptr_t)ll.begin()._ptr | fmt::FMT_HEX, (uintptr_t)ll.end()._ptr | fmt::FMT_HEX);
                        ;

                        core::forEachIdx(ll, [&](auto &val, int idx)
                                         { log::log$("[{}] = {}", idx, val); });

                        return "v[i] != i";
                    }
                    i++;
                }

                if (ll.count() != 16 || i != 16)
                {
                    log::log$("{} : {}", ll.count(), i);
                    return "count != 16";
                }

                return {};
            }),
        Test(

            "llist push/pop",
            []() -> Test::RetFn
            {
                auto ll = core::LinkedList<int>();

                for (int i = 0; i < 16; i++)
                {
                    ll.push(i);
                    if (i % 2 != 0)
                    {
                        ll.remove(ll.count() - 1);
                    }
                }
                if (ll.count() != 8)
                {
                    return "ll.count() != 8";
                }

                int i = 0;

                for (auto &v : ll)
                {
                    if (v != i * 2)
                    {
                        log::log$("[{}] = {}", i, v);

                        log::log$("{} : {}", (uintptr_t)ll.begin()._ptr | fmt::FMT_HEX, (uintptr_t)ll.end()._ptr | fmt::FMT_HEX);
                        core::forEachIdx(ll, [&](auto &val, int idx)
                                         {
                            if(idx < 100)
                            {
                                log::log$("[{}] = {}", idx, val);
                            } });

                        return "v[i/2] != i";
                    }
                    i++;
                }

                return {};
            }),

        Test(
            "llist random set",
            []() -> Test::RetFn
            {
                auto v = core::LinkedList<int>();

                for (int i = 0; i < 4096; i++)
                {
                    v.push(i);
                }
                bool has_issue = false;
                core::forEachIdx(v, [&](auto &v, int i)
                                 {
                    if(v != i)
                    {
                        has_issue = true;
                    } });

                if (has_issue)
                {
                    return "v[i] != i";
                }

                v.release();

                if (v.count() != 0)
                {
                    return "v not cleared correctly";
                }

                return {};
            }), ),
};
