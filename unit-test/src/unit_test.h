#pragma once
#include <stdio.h>
typedef int (*unit_test_func)();
int add_test(unit_test_func func);

struct unit_test
{
    unit_test_func func;

    unit_test(unit_test_func f)
    {
        func = f;
        add_test(f);
    }
};

struct unit_test_check
{
    bool status;
    const char *check_name;
    const char *error;
    bool created;
    bool logged = false;
    void echo_out()
    {
        if (created || logged)
        {
            return;
        }
        logged = true;
        printf(" %s :  ", check_name);
        if (status)
        {

            printf("\033[32msuccess\033[0m \n");
        }
        else
        {

            printf("\033[31;1;4m error (%s) \033[0m \n", error);
        }
    }

    void set_error()
    {
        status = false;
    }
};

int run_all_test();

#define DEBUG_OUT_LAST_SEQUENCE \
    __check__.echo_out();       \
    if (!__check__.status)      \
    {                           \
        return -1;              \
    };

#define LIB(file_name)                                                \
    int test##file_name##func()                                       \
    {                                                                 \
        printf("\n\n \033[94m * === %s === *\033[0m \n", #file_name); \
        unit_test_check __check__ = {true, #file_name, "no_error", true, true};

#define END_LIB(file_name)  \
    DEBUG_OUT_LAST_SEQUENCE \
    return 0;               \
    }                       \
    ;                       \
    static auto test##file_name##funcr = unit_test(test##file_name##func);

#define SECTION(class_name) \
    DEBUG_OUT_LAST_SEQUENCE \
    printf("\033[96m === %s === \033[0m\n", class_name);

#define MEMBER(name)        \
    DEBUG_OUT_LAST_SEQUENCE \
    printf("\033[96m === %s === \033[0m\n", name);

#define CHECK(name)                       \
    DEBUG_OUT_LAST_SEQUENCE               \
    __check__ = {true, name, "no_error"}; \
    __check__.logged = false;             \
    __check__.created = false;

#define REQUIRE(condition)                   \
    {                                        \
        bool condition_result = (condition); \
        if (!condition_result)               \
        {                                    \
            __check__.status = false;        \
            __check__.error = #condition;    \
            __check__.logged = false;        \
            __check__.echo_out();            \
            return -1;                       \
        }                                    \
    }

#define REQUIRE_EQUAL(value, expected_value)                 \
    {                                                        \
        bool condition_result = (value) == (expected_value); \
        if (!condition_result)                               \
        {                                                    \
            __check__.status = false;                        \
            __check__.error = #value " != " #expected_value; \
            __check__.logged = false;                        \
            __check__.echo_out();                            \
            return -1;                                       \
        }                                                    \
    }
