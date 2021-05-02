#include "unit_test.h"
#include <stdio.h>
#include <string.h>
#include <utils/container/wvector.h>
const char *last_lib_target = "null";
const char *last_target = "null";
const char *last_test = "null";

utils::vector<unit_test_func> *test_list = nullptr;

/*
int run_test(unit_test *test)
{
    if (strcmp(test->lib_target, last_lib_target) != 0)
    {
        printf("\n\n \033[94m * === lib %s === *\033[0m \n", test->lib_target);
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
*/
int add_test(unit_test_func func)
{
    if (test_list == nullptr)
    {
        test_list = new utils::vector<unit_test_func>();
    }
    test_list->push_back(func);
    return 0;
}

int run_all_test()
{
    int error_count = 0;
    int test_count = 0;

    for (size_t i = 0; i < test_list->size(); i++)
    {
        int res = test_list->get(i)();
        if (res != 0)
        {

            error_count++;
        }
        test_count++;
    }
    printf("ran %i/%i test \n", test_count - error_count, test_count);
    return (error_count);
}
// this is just for testing unit test (yes this is weird but it work)

LIB(unit_test)
{

    SECTION("test_for_unit_test")
    {
        int value = 0;

        CHECK("true condition test")
        {
            value++;
            REQUIRE_EQUAL(value, 1);
        }

        MEMBER("operator ++")
        {
            CHECK("positive result")
            {
                value = 1;
                REQUIRE_EQUAL(value, 1);
            }
            CHECK("negative result")
            {

                REQUIRE_EQUAL(value - 1, 0);
            }
        }
    }
}
END_LIB(unit_test)
