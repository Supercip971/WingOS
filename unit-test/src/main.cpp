
#include "alloc_array_check.h"
#include "array_check.h"
#include "math_check.h"
#include "stdlib_check.h"
#include "string_check.h"
#include "unit_test.h"
#include "vector_check.h"
#include <plug/system_plug.h>
#include <stdio.h>
#include <string.h>
int test()
{
    return 1;
}
int test2()
{
    return 0;
}
unit_test v[] = {
    {"(libc) string.h", "strlen", "check for strlen good return", strlen_check_0},

    {"(libc) string.h", "strnlen", "check for good return", strnlen_check_0},
    {"(libc) string.h", "strnlen", "check for clamp", strnlen_check_1},

    {"(libc) string.h", "strcmp", "check for true return", strcmp_check_true},
    {"(libc) string.h", "strcmp", "check for false return", strcmp_check_false},

    {"(libc) string.h", "memcmp", "check for true return", memcmp_check_true},
    {"(libc) string.h", "memcmp", "check for false return", memcmp_check_false},
    {"(libc) string.h", "memset", "check for good memset", memset_check_0},

    {"(libc) stdlib.h", "abs", "check for good return", abs_check},

    {"(utils) math.h", "max", "check for good return", max_check_utils},
    {"(utils) math.h", "min", "check for good return", min_check_utils},

    {"(utils) warray.h", "array", "creation test", array_creation_check},
    {"(utils) warray.h", "array", "access test", array_access_check},
    {"(utils) warray.h", "array", "fill test", array_fill_check},

    {"(utils) alloc_array.h", "alloc_array", "creation test", alloc_array_creation_test},
    {"(utils) alloc_array.h", "alloc_array", "creation fill test", alloc_array_creation_fill_test},
    {"(utils) alloc_array.h", "alloc_array", "alloc test", alloc_array_alloc_test},
    {"(utils) alloc_array.h", "alloc_array", "free test", alloc_array_free_test},
    {"(utils) alloc_array.h", "alloc_array", "foreach test", alloc_array_foreachentry_test},

    {"(utils) wvector.h", "vector", "creation test", wvector_create_check},
    {"(utils) wvector.h", "vector", "capacity test", wvector_capacity_check},
    {"(utils) wvector.h", "vector", "push back test", wvector_push_back_check},
    {"(utils) wvector.h", "vector", "get test", wvector_get_check},
    {"(utils) wvector.h", "vector", "remove test", wvector_remove_check},
    {"(utils) wvector.h", "vector", "clear test", wvector_clear_check},

};
int main(int argc, char **argv)
{
    int error_count = 0;
    int test_count = 0;
    plug::init();
    for (size_t i = 0; i < sizeof(v) / sizeof(v[0]); i++)
    {
        if (run_test(&v[i]) != 0)
        {
            error_count++;
            return error_count;
        }
        test_count++;
    }
    printf("runned %i/%i test \n", test_count - error_count, test_count);
    if (error_count != 0)
    {
        return (error_count);
    }
    return 0;
}
