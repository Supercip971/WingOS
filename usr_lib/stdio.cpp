#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#include <klib/kernel_util.h>
#include <klib/string_util.h>
char temp_buf[64];
int printf(const char* format, ...){
    va_list parameters;
    va_start(parameters, format);

    int written = 0;

    while (*format != '\0')
    {
        uint64_t maxrem = 0xffffffffff - written;

        if (format[0] != '%' || format[1] == '%')
        {
            if (format[0] == '%')
                format++;
            uint64_t amount = 1;
            while (format[amount] && format[amount] != '%')
                amount++;
            if (maxrem < amount)
            {
                // TODO: Set errno to EOVERFLOW.
                return -1;
            }
            if (!sys::write_console(format, amount))
                return written;
            format += amount;
            written += amount;
            continue;
        }

        const char *format_begun_at = format++;

        if (*format == 'c')
        {
            format++;
            char c = (char)va_arg(parameters, int /* char promotes to int */);
            if (!maxrem)
            {
                // TODO: Set errno to EOVERFLOW.
                return -1;
            }
            if (!sys::write_console(&c, sizeof(c)))
                return 0;
            written++;
        }
        else if (*format == 's')
        {
            format++;
            const char *str = va_arg(parameters, const char *);
            uint64_t len = strlen(str);
            if (maxrem < len)
            {
                // TODO: Set errno to EOVERFLOW.
                return -1;
            }
            if (!sys::write_console(str, len))
                return -1;
            written += len;
        }
        else if (*format == 'x')
        {
            format++;
            uint64_t d = va_arg(parameters, uint64_t);
            if (d == 0)
            {
                sys::write_console("0", 1);
                written += 1;
            }
            else
            {
                for (int i = 0; i < 64; i++)
                {
                    temp_buf[i] = 0;
                }
                sys::int_to_string<uint64_t>(temp_buf, 'x', d);
                uint64_t len = strlen(temp_buf);
                if (maxrem < len)
                {
                    // TODO: Set errno to EOVERFLOW.
                    return -1;
                }
                if (!sys::write_console(temp_buf, len))
                    return -1;
                written += len;
            }
        }
        else
        {
            format = format_begun_at;
            uint64_t len = strlen(format);
            if (maxrem < len)
            {
                // TODO: Set errno to EOVERFLOW.
                return -1;
            }
            if (!sys::write_console(format, len))
                return -1;
            written += len;
            format += len;
        }
    }

    va_end(parameters);
    return written;
}
