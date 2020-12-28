#include "io_device.h"
#include <logging.h>
io_device *io_list[16];
io_device::io_device()
{
}

io_device *get_io_device(uint64_t id)
{
    return io_list[id];
}
void set_io_device(io_device *dev, uint64_t id)
{
    log("io device", LOG_INFO) << "setting io device " << dev->get_io_device_name() << "id " << id;
    io_list[id] = dev;
}

io_rw_output io_device::read(uint8_t *data, uint64_t count, uint64_t cursor)
{
    log("io device", LOG_WARNING) << get_io_device_name() << "don't support reading";
    return io_rw_output::io_ERROR;
}
io_rw_output io_device::write(uint8_t *data, uint64_t count, uint64_t cursor)
{
    log("io device", LOG_WARNING) << get_io_device_name() << "don't support writing";
    return io_rw_output::io_ERROR;
}
