
#include <libc/printer.h>
#include <stdint.h>
#include <string.h>

#include "stdio.h"

int print_part(PrintTarget *targ, const char *data, size_t data_len)
{
    if (targ->is_file)
    {
        fwrite((void *)(data), 1, data_len, targ->file);
    }
    else
    {
        if (targ->buffer.cursor + data_len >= targ->buffer.len)
        {
            data_len = targ->buffer.len - targ->buffer.cursor - 1;
        }
        if (data_len > 0)
        {
            memcpy(targ->buffer.buf + targ->buffer.cursor, data, data_len);
            targ->buffer.cursor += data_len;
        }
    }

    return data_len;
}

struct PrintIntegerFlags
{
    int base;
    bool uppercase;
    int is_signed;
    char pad_char;
    int width;
    int precision;
};

int print_integer(PrintTarget *targ, unsigned long long value, struct PrintIntegerFlags flags)
{
    char buf[64] = {0};
    int digit_count = 0;
    char *ptr = buf + sizeof(buf) - 1;
    *ptr = '\0';
    ptr--;

    int negative = 0;
    if (flags.is_signed && (long long)value < 0)
    {
        negative = 1;
        value = -(long long)value;
    }
    
    if (value == 0)
    {
        *ptr = '0';
        ptr--;
        digit_count = 1;
    }
    else
    {
        while (value > 0)
        {
            int digit = value % flags.base;
            if (digit < 10)
            {
                *ptr = '0' + digit;
            }
            else
            {
                *ptr = (flags.uppercase ? 'A' : 'a') + (digit - 10);
            }
            ptr--;
            digit_count++;
            value /= flags.base;
        }
    }
    
    // Pad with zeros for precision
    while (digit_count < flags.precision)
    {
        *ptr = '0';
        ptr--;
        digit_count++;
    }

    if (negative)
    {
        *ptr = '-';
        ptr--;
    }

    // Pad with pad_char for width
    int len = strlen(ptr + 1);
    while (len < flags.width)
    {
        *ptr = flags.pad_char;
        ptr--;
        len++;
    }

    return print_part(targ, ptr + 1, strlen(ptr + 1));
}

int print_impl(PrintTarget targ, const char *fmt, va_list va)
{
    const char *p = fmt;

    size_t l = 0;
    while (*p)
    {
        struct PrintIntegerFlags flags = {
            .base = 10,
            .uppercase = false,
            .is_signed = true,
            .pad_char = ' ',
            .width = 0,
            .precision = -1};
        const char *start = p;

        while (*p && *p != '%')
        {
            p++;
        }
        if (p > start)
        {
            l += print_part(&targ, start, p - start);
        }

        if (*p != '%')
        {
            break;
        }

        p++;

        // skip flags for now
        while (*p == '-' || *p == '+' || *p == ' ' || *p == '#' || *p == '0')
        {
            if(*p == '0')
            {
                flags.pad_char = '0';
            }
            p++;
        }


        while (*p >= '0' && *p <= '9')
        {
            flags.width = flags.width * 10 + (*p - '0');

            p++;
        }

        if (*p == '.')
        {
            p++;
            flags.precision = 0;
            while (*p >= '0' && *p <= '9')
            {
                flags.precision = flags.precision * 10 + (*p - '0');
                p++;
            }
        }

        switch (*p)
        {
        case 's':
        {
            const char *str = va_arg(va, const char *);
            size_t len = strlen(str);

            if (flags.precision >= 0 && (size_t)flags.precision < len)
            {
                len = flags.precision;
            }
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
        case 'd':
        case 'i':
        {
            l += print_integer(&targ, (long long)va_arg(va, int), flags);
            p++;
            break;
        }
        case 'u':
        {
            flags.is_signed = 0;
            l += print_integer(&targ, (long long)va_arg(va, unsigned int), flags);
            p++;
            break;
        }
        case 'X':
        case 'x':
        {
            flags.base = 16;
            flags.is_signed = 0;
            flags.uppercase = (*p == 'X');
            l += print_integer(&targ, (long long)va_arg(va, unsigned int), flags);
            p++;
            break;
        }

        case 'p':
        {

            void *ptr = va_arg(va, void *);
            flags.base = 16;
            flags.is_signed = 0;

            l += print_part(&targ, "0x", 2);
            l += print_integer(&targ, (uintptr_t)ptr, flags);
            p++;
            break;
        }
        case 'l':
        {
            // Handle %ld, %li, %lu, %lx, %lX
            p++;
            if (*p == 'd' || *p == 'i')
            {
                l += print_integer(&targ, (long long)va_arg(va, long), flags);
                p++;
            }
            else if (*p == 'u')
            {
                flags.is_signed = 0;
                l += print_integer(&targ, (long long)va_arg(va, unsigned long), flags);
                p++;
            }
            else if (*p == 'x' || *p == 'X')
            {
                flags.base = 16;
                flags.is_signed = 0;
                flags.uppercase = (*p == 'X');

                l += print_integer(&targ, (long long)va_arg(va, unsigned long), flags);
                p++;
            }
            break;
        }
        default:
            // Unknown format specifier, skip it
            p++;
            break;
        }
    }

    // Null-terminate buffer output
    if (!targ.is_file && targ.buffer.cursor < targ.buffer.len)
    {
        targ.buffer.buf[targ.buffer.cursor] = '\0';
    }

    return l;
}
