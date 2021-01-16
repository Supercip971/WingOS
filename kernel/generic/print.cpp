#include "print.h"

#include <arch.h>
#include <general_device.h>
#include <stdarg.h>
#include <utility.h>
lock_type print_locker = {0};
char temp_buf[64];
void printf(const char *format, ...)
{

    lock((&print_locker));
    va_list parameters;
    va_start(parameters, format);
    debug_device *dev = find_device<debug_device>();
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
                unlock((&print_locker));
                return;
            }
            if (!dev->echo_out(format, amount))
            {

                unlock((&print_locker));
                return;
            }
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
                unlock((&print_locker));
                return;
            }
            if (!dev->echo_out(&c, sizeof(c)))
            {

                unlock((&print_locker));
                return;
            }
            written++;
        }
        else if (*format == 's')
        {
            format++;
            const char *str = va_arg(parameters, const char *);
            uint64_t len = strlen(str);
            if (maxrem < len)
            {

                unlock((&print_locker));
                return;
            }
            if (!dev->echo_out(str, len))
            {

                unlock((&print_locker));
                return;
            }
            written += len;
        }
        else if (*format == 'x')
        {
            format++;
            uint64_t d = va_arg(parameters, uint64_t);
            if (d == 0)
            {
                dev->echo_out("0", 1);
                written += 1;
            }
            else
            {
                for (int i = 0; i < 64; i++)
                {
                    temp_buf[i] = 0;
                }
                kitoaT(temp_buf, 'x', d);
                uint64_t len = strlen(temp_buf);
                if (maxrem < len)
                {
                    unlock((&print_locker));
                    // TODO: Set errno to EOVERFLOW.
                    return;
                }
                if (!dev->echo_out(temp_buf, len))
                {

                    unlock((&print_locker));
                    return;
                }
                written += len;
            }
        }
        else
        {
            format = format_begun_at;
            uint64_t len = strlen(format);
            if (maxrem < len)
            {

                unlock((&print_locker));
                return;
            }
            if (!dev->echo_out(format, len))
            {

                unlock((&print_locker));
                return;
            }
            written += len;
            format += len;
        }
    }

    va_end(parameters);
    unlock((&print_locker));
    return;
}
