#include <arch/arch.h>
#include <com.h>
#include <kernel.h>
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

char temp_buffer[64];
void com_write_reg(const char *buffer, uint64_t value)
{
    for (uint64_t tb = 0; tb < 64; tb++)
    {
        temp_buffer[tb] = 0;
    }
    kitoaT(temp_buffer, 'x', value);
    com_write_strl(buffer);
    com_write_str(temp_buffer);
}
