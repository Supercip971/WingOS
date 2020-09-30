#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#include <klib/kernel_util.h>
#include <klib/string_util.h>
char temp_buf[64];


int vsn_printf_out(bool just_print, char* buffer, uint64_t count, const char* data ){
    if(just_print == true){
        sys::write_console(data, count);
        return 1;
    }else if(buffer != nullptr){
        strncpy(buffer, data, count);
        buffer = buffer + count;
        return 1;
    }
    return 0;
}
int vsn_printf(bool just_print, char* buffer /* nullptr say no buffer just print it*/, uint64_t count, const char* format, va_list argument){


    size_t written = 0;

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
            if (!vsn_printf_out(just_print,buffer, amount, format))
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
            if (!vsn_printf_out(just_print,buffer, sizeof (c), &c))
                return 0;
            written++;
        }
        else if (*format == 's')
        {
            format++;
            const char *str = va_arg(argument, const char *);
            uint64_t len = strlen(str);
            if (maxrem < len)
            {
                // TODO: Set errno to EOVERFLOW.
                return -1;
            }
            if (!vsn_printf_out(just_print,buffer, len, str))
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
                vsn_printf_out(just_print,buffer, sizeof (c), &c);
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
                if (!vsn_printf_out(just_print,buffer, len, temp_buf))
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
            if (!vsn_printf_out(just_print,buffer, len, format))
                return -1;
            written += len;
            format += len;
        }
    }
    return written;
}
int sprintf(char* buffer, const char* format, ...){

    va_list va;
    va_start(va, format);
    uint64_t return_value = vsn_printf(false, buffer, 0, format, va);
    va_end((va));
    return return_value;

}
int printf(const char* format, ...){
    va_list va;
    va_start(va, format);
    uint64_t return_value = vsn_printf(true, nullptr, 0, format, va);
    va_end((va));
    return return_value;
}
