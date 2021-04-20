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

    zero_interrupt_identificator();
    turn_on_dlab();
    set_baud(3);
    set_data_size(COM_LINE_CONTROL_BIT::COM_DATA_SIZE_8); // 8 bit
    fill_interrupt_identificator();
    write_modem(COM_MODEM_DTR | COM_MODEM_RTS | COM_MODEM_OUT2);

    add_device(this);
}
void com_device::wait() const
{
    int timeout = 0;
    while ((read(COM_REGISTER::COM_LINE_STATUS) & 0x20) == 0)
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
    write(COM_REGISTER::COM_DATA, c);
}
