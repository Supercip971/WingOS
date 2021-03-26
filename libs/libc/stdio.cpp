
#include <plug/system_plug.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <utils/string_util.h>

char temp_buf[64];

FILE *stdin;
FILE *stdout;
FILE *stderr;

int vsn_printf_out(bool just_print, char *buffer, uint64_t count, const char *data)
{
    if (just_print == true)
    {
        plug_debug_out(data, count);
        return 1;
    }
    else if (buffer != nullptr)
    {
        strncpy(buffer, data, count);
        buffer = buffer + count;
        return 1;
    }
    return 0;
}

int vsn_printf(bool just_print, char *buffer /* nullptr say no buffer just print it*/, uint64_t count, const char *format, va_list argument)
{
    size_t written = 0;
    while (*format != '\0')
    {
        size_t maxrem = 0xffffffffff - written;

        if (format[0] != '%' || format[1] == '%')
        {
            if (format[0] == '%')
                format++;
            size_t amount = 1;
            while (format[amount] && format[amount] != '%')
                amount++;
            if (maxrem < amount)
            {
                // TODO: Set errno to EOVERFLOW.
                return -1;
            }
            if (!vsn_printf_out(just_print, buffer, amount, format))
                return written;
            format += amount;
            written += amount;
            continue;
        }

        const char *format_begun_at = format++;

        if (*format == 'c')
        {
            format++;
            char c = (char)va_arg(argument, int /* char promotes to int */);
            if (!maxrem)
            {
                // TODO: Set errno to EOVERFLOW.
                return -1;
            }
            if (!vsn_printf_out(just_print, buffer, sizeof(c), &c))
                return 0;
            written++;
        }
        else if (*format == 's')
        {
            format++;
            const char *str = va_arg(argument, const char *);
            size_t len = strlen(str);
            if (maxrem < len)
            {
                // TODO: Set errno to EOVERFLOW.
                return -1;
            }
            if (!vsn_printf_out(just_print, buffer, len, str))
                return -1;
            written += len;
        }
        else if (*format == 'x')
        {
            format++;
            uint64_t d = va_arg(argument, uint64_t);
            if (d == 0)
            {
                char c = '0';
                vsn_printf_out(just_print, buffer, sizeof(c), &c);
                written += 1;
            }
            else
            {
                for (int i = 0; i < 64; i++)
                {
                    temp_buf[i] = 0;
                }
                utils::int_to_string<uint64_t>(temp_buf, 'x', d);
                size_t len = strlen(temp_buf);
                if (maxrem < len)
                {
                    // TODO: Set errno to EOVERFLOW.
                    return -1;
                }
                if (!vsn_printf_out(just_print, buffer, len, temp_buf))
                    return -1;
                written += len;
            }
        }
        else if (*format == 'i')
        {
            format++;
            int d = va_arg(argument, int);
            if (d == 0)
            {
                char c = '0';
                vsn_printf_out(just_print, buffer, sizeof(c), &c);
                written += 1;
            }
            else
            {
                for (int i = 0; i < 64; i++)
                {
                    temp_buf[i] = 0;
                }
                utils::int_to_string<int>(temp_buf, 'd', d);
                size_t len = strlen(temp_buf);
                if (maxrem < len)
                {
                    // TODO: Set errno to EOVERFLOW.
                    return -1;
                }
                if (!vsn_printf_out(just_print, buffer, len, temp_buf))
                    return -1;
                written += len;
            }
        }
        else if (*format == 'l')
        {
            format++;
            long d = va_arg(argument, long);
            if (d == 0)
            {
                char c = '0';
                vsn_printf_out(just_print, buffer, sizeof(c), &c);
                written += 1;
            }
            else
            {
                for (int i = 0; i < 64; i++)
                {
                    temp_buf[i] = 0;
                }
                utils::int_to_string<long>(temp_buf, 'd', d);
                size_t len = strlen(temp_buf);
                if (maxrem < len)
                {
                    // TODO: Set errno to EOVERFLOW.
                    return -1;
                }
                if (!vsn_printf_out(just_print, buffer, len, temp_buf))
                    return -1;
                written += len;
            }
        }
        else
        {
            format = format_begun_at;
            size_t len = strlen(format);
            if (maxrem < len)
            {
                // TODO: Set errno to EOVERFLOW.
                return -1;
            }
            if (!vsn_printf_out(just_print, buffer, len, format))
                return -1;
            written += len;
            format += len;
        }
    }
    return written;
}

int sprintf(char *buffer, const char *format, ...)
{
    va_list va;
    va_start(va, format);
    int return_value = vsn_printf(false, buffer, 0, format, va);
    va_end((va));
    return return_value;
}

int printf(const char *format, ...)
{
    va_list va;
    va_start(va, format);
    int return_value = vsn_printf(true, nullptr, 0, format, va);
    va_end((va));
    return return_value;
}

int vsprintf(char *buffer, const char *format, va_list vlist)
{
    int return_value = vsn_printf(false, buffer, 0, format, vlist);
    return return_value;
}

FILE *fopen(const char *pathname, const char *mode)
{
    FILE *f = (FILE *)malloc(sizeof(FILE));
    f->file_element = plug_open(pathname, 0, 0);
    return f;
}
int fclose(FILE *stream)
{
    plug_close(stream->file_element);
    free(stream);
    return 0;
}

int fseek(FILE *stream, long offset, int whence)
{
    return plug_lseek(stream->file_element, offset, whence);
}

long ftell(FILE *stream)
{
    return fseek(stream, 0, SEEK_CUR);
}

size_t fread(void *ptr, size_t size, size_t count, FILE *stream)
{
    return plug_read(stream->file_element, ptr, size * count);
}

int fgetc(FILE *stream)
{
    char off = 0;
    fread(&off, sizeof(char), 1, stream);
    return off;
}

int ungetc(int c, FILE *stream)
{
    return 0;
}

int feof(FILE *stream)
{
    if (fgetc(stream) == EOF)
    {
        return true;
    }
    return false;
}

int ferror(FILE *stream)
{
    return true; // error not supported for the moment
}
