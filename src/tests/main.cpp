#include <libcore/fmt/log.hpp>

#include "libcore/grp_tests.hpp"
#include "unit-test-tests/grp_tests.hpp"

#include "libcore/result.hpp"
#include "test.hpp"

TestGroup v = {
    test_grouped_group$(
        "tests",
        libcoreTests,
        unitTestTests),

};

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    log::log$("Hello, World!");

    v.execute(0);
    return 0;
}
