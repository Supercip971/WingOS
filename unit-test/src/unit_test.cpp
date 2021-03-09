#include "unit_test.h"
#include <stdio.h>
#include <string.h>

const char *last_lib_target = "null";
const char *last_target = "null";
const char *last_test = "null";

int run_test(unit_test *test)
{
    if (strcmp(test->lib_target, last_lib_target) != 0)
    {
        printf("\033[94m * === lib %s === *\033[0m \n", test->lib_target);
        last_lib_target = test->lib_target;
    }
    if (strcmp(test->target, last_target) != 0)
    {
        printf("\033[96m === %s === \033[0m \n", test->target);
        last_target = test->target;
    }
    printf("testing %s ... ", test->test);

    int res = test->func();
    if (res != 0)
    {
        printf("\033[31;1;4mERROR %i\033[0m \n", res);
        return res;
    }
    else
    {
        printf("\033[32msuccess\033[0m \n");
        return 0;
    }
}