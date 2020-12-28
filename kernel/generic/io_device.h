#ifndef IO_DEVICE_H
#define IO_DEVICE_H
#include <stdint.h>
enum io_rw_output
{
    io_ERROR = 0,
    io_OK = 1,
};

class io_device
{
public:
    virtual void init(){};
    virtual io_rw_output read(uint8_t *data, uint64_t count, uint64_t cursor);
    virtual io_rw_output write(uint8_t *data, uint64_t count, uint64_t cursor);
    virtual const char *get_io_device_name()
    {
        return "null io_device";
    }
    io_device();
};
io_device *get_io_device(uint64_t id);
void set_io_device(io_device *dev, uint64_t id);
#endif // IO_DEVICE_H
