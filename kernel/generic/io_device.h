#pragma once
#include <stdint.h>
enum io_rw_output
{
    io_ERROR = 0,
    io_OK = 1,
};

class io_device
{
public:
    virtual void init() = 0;
    virtual io_rw_output read(uint8_t *data, uint64_t count, uint64_t cursor) = 0;
    virtual io_rw_output write(uint8_t *data, uint64_t count, uint64_t cursor) = 0;
    virtual const char *get_io_device_name() = 0;
    io_device();
};
io_device *get_io_device(uint64_t id);
void set_io_device(io_device *dev, uint64_t id);
