#include <arch.h>
#include <com.h>
#include <kernel.h>
#include <process.h>
#include <stdarg.h>
#include <utility.h>
char temp_buffer[17];
uint64_t last_count = 17;

bool com_device::echo_out(const char *data, uint64_t data_length)
{

    for (uint64_t i = 0; i < data_length; i++)
    {
        write(data[i]);
    }

    return true;
}
bool com_device::echo_out(const char *data)
{

    uint64_t i = 0;
    while (data[i] != 0)
    {
        write(data[i]);
        i++;
    }
    return true;
}

void com_device::init(COM_PORT this_port)
{
    port = this_port;
    outb(port + 2, 0);
    outb(port + 3, 1 << 7);
    outb(port + 0, 3);
    outb(port + 1, 0);
    outb(port + 3, 0x03);
    outb(port + 2, 0xC7);
    outb(port + 4, 0x0B);
    add_device(this);
}
void com_device::wait() const
{
    int timeout = 0;
    while ((inb(port + 5) & 0x20) == 0)
    {
        if (timeout++ > 10000)
        {
            break;
        }
    }
}
inline void com_device::write(char c) const
{
    wait();
    outb(port, c);
}
