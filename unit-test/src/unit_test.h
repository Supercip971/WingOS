#pragma once

typedef int (*unit_test_func)();
struct unit_test
{
    const char *lib_target;
    const char *target;
    const char *test;
    unit_test_func func;
};

int run_test(unit_test *test);