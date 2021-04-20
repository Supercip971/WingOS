#pragma once
#include <64bit.h>
#include <arch.h>
#include <general_device.h>
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
