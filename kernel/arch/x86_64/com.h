#pragma once
#include <arch.h>
#include <general_device.h>
#include <int_value.h>
enum COM_PORT
{
    COM1 = 0x3F8,
    COM2 = 0x2F8,
    COM3 = 0x3E8,
    COM4 = 0x2E8,
};
///FIXME: should not exist

class com_device : public debug_device
{
    COM_PORT port;

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
