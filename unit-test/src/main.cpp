
#include <utils/memory.h>
#include "unit_test.h"
#include "string_check.h"
#include <plug/system_plug.h>
#include "math_check.h"
#include <stdio.h>
#include <string.h>
int test(){
    return 1;
}
int test2(){
    return 0;
}
unit_test v[] = { 
    {"(libc) string.h", "strlen", "check for strlen good return",  strlen_check_0}, 
    {"(libc) string.h", "strnlen", "check for good return",        strnlen_check_0},
    {"(libc) string.h", "strnlen", "check for clamp",              strnlen_check_1},
    {"(libc) string.h", "strcmp",  "check for true return",        strcmp_check_true},
    {"(libc) string.h", "strcmp",  "check for false return",        strcmp_check_false},
    {"(utils) math.h", "max",  "check for good return",        max_check_utils},
    {"(utils) math.h", "min",  "check for good return",        min_check_utils}
    };
int main(int argc, char **argv){
    int error_count = 0;
    int test_count = 0;
    plug::init();
    for(size_t i = 0; i < sizeof(v) / sizeof(v[0]); i++){
        if(run_test(&v[i]) != 0){
            error_count++;

        }
        test_count++;
    }
    printf("runned %i/%i test \n", test_count - error_count, test_count);
    if(error_count != 0){
        return (error_count);
    }
    return 0;
}