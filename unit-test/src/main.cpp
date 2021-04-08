
#include "alloc_array_check.h"
#include "array_check.h"
#include "bit_check.h"
#include "math_check.h"
#include "memory_check.h"
#include "smart_ptr_check.h"
#include "stdlib_check.h"
#include "string_check.h"
#include "type_trait_check.h"
#include "unit_test.h"
#include "vector_check.h"
#include <plug/system_plug.h>

#include <stdio.h>
#include <string.h>
bool try_to_exit = false;
int main(int argc, char **argv)
{
    plug_init();
    return run_all_test();
}
