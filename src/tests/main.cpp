#include <libcore/fmt/log.hpp>

#include "libcore/grp_tests.hpp"
#include "tests/parser/parser_test.hpp"
#include "unit-test-tests/grp_tests.hpp"

#include "libcore/result.hpp"
#include "test.hpp"
#include "tests/json/json-tests.hpp"

TestGroup v = {
    test_grouped_group$(
        "tests",
        libcoreTests,
        unitTestTests,
        parserTest,
        jsonTests),

};

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    log::log$("Hello, World!");

    v.execute(0);
    return 0;
}
