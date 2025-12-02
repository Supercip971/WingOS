#include "stdio.h"
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>

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

int print_part(PrintTarget *targ, const char *data, size_t data_len)
{
    if (targ->is_file)
    {
        fwrite((void *)(data ), 1, data_len, targ->file);
    }
    else
    {

        if(targ->buffer.cursor + data_len >= targ->buffer.len)
        {
            data_len = targ->buffer.len - targ->buffer.cursor - 1;

        }
        strncpy(targ->buffer.buf + targ->buffer.cursor, data, data_len);
    }

    return data_len;
}
int scan_impl(PrintTarget targ, const char *fmt, va_list va)
{
    size_t l = 0;
    const char *p = fmt;
    while (*p )
    {
        const char* start = p;

        while(*p && *p != '%')
        {
            p++;
        }
        if(p > start)
        {
            l += print_part(&targ, start, p - start);
        }
        if(*p == '%')
        {
            p++;
            switch(*p)
            {
                case 's':
                {
                    char* str = va_arg(va, char*);
                    size_t len = 0;
                    while(targ.buffer.buf[targ.buffer.cursor + len] != ' ' && targ.buffer.buf[targ.buffer.cursor + len] != '\0')
                    {
                        len++;
                    }
                    strncpy(str, targ.buffer.buf + targ.buffer.cursor, len);
                    targ.buffer.cursor += len;
                    l += len;
                    p++;
                    break;
                }
                case 'c':
                {
                    char* c = va_arg(va, char*);
                    *c = targ.buffer.buf[targ.buffer.cursor];
                    targ.buffer.cursor += 1;
                    l += 1;
                    p++;
                    break;
                }
                case 'i':
                {
                    int* i = va_arg(va, int*);
                    // use strtoll
                    long long val = strtoll(targ.buffer.buf + targ.buffer.cursor, NULL, 10);

                    *i = (int)val;

                   
                    break;
                }
                case 'x':
                {
                    int* i = va_arg(va, int*);

                    long long val = strtoll(targ.buffer.buf + targ.buffer.cursor, NULL, 16);
                    *i = (int)val;
                  
                    break;
                }
                default:
                    // Unsupported format specifier, just skip
                    p++;
                    break;
            }
        }
    }

    return l ; 
}
int print_impl(PrintTarget targ, const char *fmt, va_list va)
{
    const char *p = fmt;

    size_t l = 0;
    while (*p )
    {
        const char* start = p;

        while(*p && *p != '%')
        {
            p++;
        }
        if(p > start)
        {
            l += print_part(&targ, start, p - start);
        }
        if(*p == '%')
        {
            p++;
            switch(*p)
            {
                case 's':
                {
                    const char* str = va_arg(va, const char*);
                    size_t len = strlen(str);
                    l += print_part(&targ, str, len);
                    p++;
                    break;
                }
                case 'c':
                {
                    char c = (char)va_arg(va, int);
                    l += print_part(&targ, &c, 1);
                    p++;
                    break;
                }
                case '%':
                {
                    char c = '%';
                    l += print_part(&targ, &c, 1);
                    p++;
                    break;
                }
                default:
                    l += print_part(&targ, p, 1);
                    // Unsupported format specifier, just skip
                    p++;
                    break;
            }
        }
    }


    return l;
}

int snprintf(char *s, size_t n, const char *format, ...)
{
    va_list lst;

    va_start(lst, format);

    PrintTarget targ;
    targ.is_file = false;
    targ.buffer.buf = s;
    targ.buffer.len = n;
    targ.buffer.cursor = 0;
    int l = print_impl(targ, format, lst);
    va_end(lst);
    return l;
}
int vfprintf(FILE *__restrict stream, const char *__restrict format, va_list arg)
{
    PrintTarget targ;
    targ.is_file = true;
    targ.file = stream;
    int l = print_impl(targ, format, arg);
    return l;
}

int putchar(int c)
{
    char ch = (char)c;
    fwrite((void*)&ch, 1, 1, stdout);
    return c;
}
int fprintf(FILE *__restrict stream, const char *__restrict format, ...)
{
    va_list lst;

    va_start(lst, format);

    PrintTarget targ;
    targ.is_file = true;
    targ.file = stream;
    int l = print_impl(targ, format, lst);
    va_end(lst);
    return l;
}
int printf( const char *__restrict format, ...)
{
    va_list lst;

    va_start(lst, format);

    PrintTarget targ;
    targ.is_file = true;
    targ.file = stdout;
    int l = print_impl(targ, format, lst);
    va_end(lst);
    return l;
}

int sscanf(const char* __restrict str, const char* __restrict format, ...)
{
    va_list lst;

    va_start(lst, format);

    PrintTarget targ;
    targ.is_file = false;
    targ.buffer.buf = (char*)str;
    int l = print_impl(targ, format, lst);
    va_end(lst);
    return l;
}

