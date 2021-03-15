
#include "alloc_array_check.h"
#include "array_check.h"
#include "math_check.h"
#include "smart_ptr_check.h"
#include "stdlib_check.h"
#include "string_check.h"
#include "unit_test.h"
#include "vector_check.h"
#include "type_trait_check.h"
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

    {"(utils) integral_constant.h", "integral_constant", "check for valid class value", integral_constant_check },
    {"(utils) integral_constant.h", "true_type", "check for valid true constant value", integral_constant_false_check },
    {"(utils) integral_constant.h", "false_type", "check for valid false constant value", integral_constant_true_check },
    
    {"(utils) type/is_same.h", "is_same", "check for true return", is_same_check_true},
    {"(utils) type/is_same.h", "is_same", "check for false return", is_same_check_false},
    
    {"(utils) type_traits.h", "remove_reference", "check for good return", remove_reference_check },
    {"(utils) type_traits.h", "remove_const", "check for good return", remove_const_check },
    {"(utils) type_traits.h", "remove_volatile", "check for good return", remove_volatile_check },
    {"(utils) type_traits.h", "remove_pointer", "check for good return", remove_pointer_check },

    {"(utils) smart_ptr.h", "unique_ptr", "create/destroy test", unique_ptr_create_destroy_check},
    {"(utils) smart_ptr.h", "unique_ptr", "raw() test", unique_ptr_raw_check},
    {"(utils) smart_ptr.h", "unique_ptr", "operator test", unique_ptr_operator_check},
    {"(utils) smart_ptr.h", "unique_ptr", "reset test", unique_ptr_reset_check},
    {"(utils) smart_ptr.h", "unique_ptr", "release test", unique_ptr_release_check},
    {"(utils) smart_ptr.h", "make unique", "make unique test", make_unique_check},

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
