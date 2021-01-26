#include "general_device.h"
#include <liballoc.h>
#include <logging.h>
#include <utility.h>
uint32_t current_array_count = 0;
general_device *device_array[MAX_DEVICE];
const char *device_type_to_str[] = {
    "timer",
    "drive",
    "network",
    "mouse",
    "keyboard",
    "clock",
    "debug",
};
const char *general_device::get_name() const
{
    return "null device";
}
device_type general_device::get_type() const
{
    return device_type::NULL_DEVICE;
}

void add_device(general_device *dev)
{
    device_array[current_array_count] = dev;
    dev->device_id = current_array_count;
    current_array_count++;
    log("device", LOG_INFO) << "added device " << current_array_count - 1 << " ' " << dev->get_name() << " ' type : " << device_type_to_str[dev->get_type()];
}

general_device *get_device(uint32_t id)
{
    return device_array[id];
}

uint32_t get_device_count()
{
    return current_array_count;
}
template <class end_type>
auto find_device() -> end_type *
{
    for (uint32_t i = 0; i < current_array_count; i++)
    {
        if (device_array[i]->get_type() == end_type::get_stype())
        {
            return static_cast<end_type *>(device_array[i]);
        }
    }
    log("device", LOG_WARNING) << "no device with type " << (int)end_type::get_stype() << "founded";
    return static_cast<end_type *>(nullptr);
}
template general_keyboard *find_device();
template interrupt_timer *find_device();
template general_mouse *find_device();
template generic_io_device *find_device();
template debug_device *find_device();

generic_io_device::io_rw_output generic_io_device::read_unaligned(uint8_t *data, uint64_t count, uint64_t cursor)
{
    uint64_t max_block_count = (((cursor % 512) + (count)) / 512) + 1;
    uint8_t *raw = (uint8_t *)malloc(max_block_count * 512);
    io_rw_output r = io_rw_output::io_OK;
    for (size_t i = 0; i < max_block_count; i++)
    {

        r = read(raw + (i * 512), 1, i + (cursor / 512));
        if (r != io_rw_output::io_OK)
        {
            free(raw);
            return r;
        }
    }
    memcpy(data, raw + cursor % 512, (count));
    free(raw);
    return r;
}
generic_io_device::io_rw_output generic_io_device::write_unaligned(uint8_t *data, uint64_t count, uint64_t cursor)
{
    return generic_io_device::io_rw_output ::io_ERROR;
}
