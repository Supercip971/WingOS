#include "stdio.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "printer.h"

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
    fwrite((void *)&ch, 1, 1, stdout);
    return c;
}

int fputc(int c, FILE *stream)
{
    char ch = (char)c;
    fwrite((void *)&ch, 1, 1, stream);
    return c;
}

int putc(int c, FILE *stream)
{
    char ch = (char)c;
    fwrite((void *)&ch, 1, 1, stream);
    return c;
}

char *fgets(char *__restrict str, int n, FILE *__restrict stream)
{
    size_t i = 0;
    while (i < (size_t)(n - 1))
    {
        int c = fgetc(stream);
        if (c == EOF)
        {
            break;
        }
        str[i++] = (char)c;
        if (c == '\n')
        {
            break;
        }
    }
    str[i] = '\0';
    return str;
}

int fputs(const char *__restrict str, FILE *__restrict stream)
{
    size_t len = strlen(str);
    fwrite((void *)str, 1, len, stream);

    return (int)len;
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

int printf(const char *__restrict format, ...)
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

int sscanf(const char *__restrict str, const char *__restrict format, ...)
{
    va_list lst;

    va_start(lst, format);

    PrintTarget targ;
    targ.is_file = false;
    targ.buffer.buf = (char *)str;
    int l = print_impl(targ, format, lst);
    va_end(lst);
    return l;
}

int vsnprintf(char *__restrict s, size_t n, const char *format, va_list arg)
{
    PrintTarget targ;
    targ.is_file = false;
    targ.buffer.buf = s;
    targ.buffer.len = n;
    targ.buffer.cursor = 0;
    int l = print_impl(targ, format, arg);
    return l;
}

int sprintf(char *__restrict s, const char *__restrict format, ...)
{
    va_list lst;

    va_start(lst, format);

    PrintTarget targ;
    targ.is_file = false;
    targ.buffer.buf = s;
    int l = print_impl(targ, format, lst);
    va_end(lst);
    return l;
}
