#include <arch/arch.h>
#include <com.h>
#include <kernel.h>
#include <stdarg.h>
#include <utility.h>
#pragma GCC optimize("-O0")
inline void com_wait_write(COM_PORT port)
{
    while ((inb(port + 5) & 0x20) == 0)
    {
    }
}
void com_putc(COM_PORT port, char c)
{
    com_wait_write(port);
    outb(port, c);
}

int com_write(COM_PORT port, const void *buffer, int size)
{
    const char *bufaddr = (const char *)buffer;
    for (int i = 0; i < size; i++)
    {
        com_putc(port, (bufaddr)[i]);
    }

    return size;
}
void com_write_str(const char *buffer)
{
    int i = 0;
    while (buffer[i] != 0)
    {
        com_putc(COM_PORT::COM1, buffer[i]);
        i++;
    }

    com_putc(COM_PORT::COM1, '\n');
}
void com_write_strl(const char *buffer)
{
    int i = 0;
    while (buffer[i] != 0)
    {
        com_putc(COM_PORT::COM1, buffer[i]);
        i++;
    }
}
bool com_write_strn(const char *buffer, uint64_t lenght)
{
    for (uint64_t i = 0; i < lenght; i++)
    {
        com_putc(COM_PORT::COM1, buffer[i]);
    }

    return true; // maybe
}
void com_initialize(COM_PORT port)
{
    outb(port + 2, 0);
    outb(port + 3, 1 << 7);
    outb(port + 0, 3);
    outb(port + 1, 0);
    outb(port + 3, 0x03);
    outb(port + 2, 0xC7); // No idea what this does :/
    outb(port + 4, 0x0B); // No idea what this does either
};

char temp_buffer[17];
uint64_t last_count = 17;
void com_write_reg(const char *buffer, uint64_t value)
{
    for (uint64_t tb = 0; tb < last_count; tb++)
    {
        temp_buffer[tb] = 0;
    }
    kitoaT(temp_buffer, 'x', value);

    com_write_strl(buffer);
    com_write_str(temp_buffer);
}

uint64_t strlen(const char *d)
{
    uint64_t lenght = 0;
    while (d[lenght] != 0)
    {
        lenght++;
    }
    return lenght;
}

void printf(const char *format, ...)
{
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
                return;
            }
            if (!com_write_strn(format, amount))
                return;
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
                return;
            }
            if (!com_write_strn(&c, sizeof(c)))
                return;
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
                return;
            }
            if (!com_write_strn(str, len))
                return;
            written += len;
        }
        else if (*format == 'x')
        {
            format++;
            uint64_t d = va_arg(parameters, uint64_t);
            char temp_buf[64];
            memzero(temp_buf, 64);
            kitoa64(temp_buf, 'x', d);
            uint64_t len = strlen(temp_buf);
            if (maxrem < len)
            {
                // TODO: Set errno to EOVERFLOW.
                return;
            }
            if (!com_write_strn(temp_buf, len))
                return;
            written += len;
        }
        else
        {
            format = format_begun_at;
            uint64_t len = strlen(format);
            if (maxrem < len)
            {
                // TODO: Set errno to EOVERFLOW.
                return;
            }
            if (!com_write_strn(format, len))
                return;
            written += len;
            format += len;
        }
    }

    va_end(parameters);
    return;
}
