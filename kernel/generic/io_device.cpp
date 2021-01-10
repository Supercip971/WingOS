#include "io_device.h"
#include <logging.h>
generic_io_device *io_list[16];
uint64_t last_io_id = 0;

generic_io_device *get_io_device(uint64_t id)
{
    return io_list[id];
}
void add_io_device(generic_io_device *dev)
{
    add_device(dev);
    log("io device", LOG_INFO) << "setting io device " << dev->get_name() << "id " << last_io_id;
    io_list[last_io_id] = dev;
    last_io_id += 1;
}
