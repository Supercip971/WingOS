#include <arch/arch.h>
#include <arch/lock.h>
#include <arch/process.h>
#include <com.h>
#include <kernel.h>
#include <stdarg.h>
#include <utility.h>
lock_type locker_print = {0};
char temp_buffer[17];
uint64_t last_count = 17;

inline void com_wait_write(COM_PORT port)
{
    while ((inb(port + 5) & 0x20) == 0)
    {
    }
}
lock_type lck = {0};
void com_putc(COM_PORT port, char c)
{
    com_wait_write(port);
    outb(port, c);
}

int com_write(COM_PORT port, const void *buffer, int size)
{
    lock(&locker_print);
    const char *bufaddr = (const char *)buffer;
    for (int i = 0; i < size; i++)
    {
        com_putc(port, (bufaddr)[i]);
    }
    unlock(&locker_print);

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
    lock(&locker_print);
    for (uint64_t i = 0; i < lenght; i++)
    {
        com_putc(COM_PORT::COM1, buffer[i]);
    }
    unlock(&locker_print);

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
lock_type print_locker = {0};
char temp_buf[64];
void printf(const char *format, ...)
{

    lock((&print_locker));
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
                unlock((&print_locker));
                return;
            }
            if (!com_write_strn(format, amount))
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
            if (!com_write_strn(&c, sizeof(c)))
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
            if (!com_write_strn(str, len))
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
                com_write_strn("0", 1);
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
                if (!com_write_strn(temp_buf, len))
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
                // TODO: Set errno to EOVERFLOW.

                unlock((&print_locker));
                return;
            }
            if (!com_write_strn(format, len))
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
