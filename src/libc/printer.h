#pragma once 
#include <stdbool.h>

#include "stdio.h"
typedef struct PrintTarget
{

    bool is_file;
    union
    {

        FILE *file;
        struct
        {
            size_t len;
            char *buf;
            size_t cursor;
        } buffer;
    };
} PrintTarget;
int print_part(PrintTarget *targ, const char *data, size_t data_len);

int print_impl(PrintTarget targ, const char *fmt, va_list va);
