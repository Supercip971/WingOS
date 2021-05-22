#pragma once
#include <64bit.h>
#include <arch.h>
#include <device/general_device.h>
#include <stdint.h>
#include <utility.h>
enum COM_PORT
{
    COM1 = 0x3F8,
    COM2 = 0x2F8,
    COM3 = 0x3E8,
    COM4 = 0x2E8,
};
enum COM_REGISTER
{
    COM_DATA = 0,
    COM_INTERRUPT = 1,
    COM_INTERRUPT_IDENTIFICATOR = 2,
    COM_FIFO_CONTROLLER = 2,
    COM_LINE_CONTROL = 3,
    COM_MODEM_CONTROL = 4,
    COM_LINE_STATUS = 5,
    COM_MODEM_STATUS = 6,
    COM_SCRATCH_REGISTER = 7,
};

enum COM_LINE_CONTROL_BIT
{
    COM_DATA_SIZE_5 = 0,
    COM_DATA_SIZE_6 = 1,
    COM_DATA_SIZE_7 = 2,
    COM_DATA_SIZE_8 = 3,
    COM_DLAB_STATUS = 1 << 7,
};

enum COM_MODEM_BIT
{
    COM_MODEM_DTR = 1 << 0,
    COM_MODEM_RTS = 1 << 1,
    COM_MODEM_OUT1 = 1 << 2,
    COM_MODEM_OUT2 = 1 << 3,
    COM_MODEM_LOOPBACK = 1 << 4,
};

class com_device : public debug_device
{
    COM_PORT port;

    void write(size_t offset, uint8_t value) const
    {
        outb(offset + port, value);
    }

    uint8_t read(size_t offset) const
    {
        return inb(port + offset);
    }

    void set_data_size(COM_LINE_CONTROL_BIT size)
    {
        write(COM_REGISTER::COM_LINE_CONTROL, size);
    }

    void turn_on_dlab()
    {
        write(COM_REGISTER::COM_LINE_CONTROL, COM_LINE_CONTROL_BIT::COM_DLAB_STATUS);
    }

    void set_baud(size_t rate)
    {
        write(COM_REGISTER::COM_DATA, rate & 0xff);
        write(COM_REGISTER::COM_INTERRUPT, (rate << 8) & 0xff);
    }

    void set_interrupt(bool status)
    {
        write(COM_REGISTER::COM_INTERRUPT_IDENTIFICATOR, status);
    }

    void fill_interrupt_identificator()
    {
        write(COM_REGISTER::COM_INTERRUPT_IDENTIFICATOR, 0xC7);
    }

    void zero_interrupt_identificator()
    {
        write(COM_REGISTER::COM_INTERRUPT_IDENTIFICATOR, 0);
    }

    void write_modem(size_t value)
    {
        write(COM_REGISTER::COM_MODEM_CONTROL, value);
    }

public:
    void wait() const;
    inline void write(char c) const;

    bool echo_out(const char *data, uint64_t data_length) final;
    bool echo_out(const char *data) final;

    void init(COM_PORT this_port);

    const char *get_name() const final
    {
        return "com";
    }
};
