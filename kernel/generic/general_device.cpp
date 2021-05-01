#include "general_device.h"
#include <logging.h>
#include <utility.h>
#include <utils/memory/liballoc.h>
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

void add_device(general_device *dev)
{
    device_array[current_array_count] = dev;
    dev->device_id = current_array_count;
    current_array_count++;
    log("device", LOG_INFO, "added device {}: '{}', type: {}", current_array_count - 1, dev->get_name(), device_type_to_str[dev->get_type()]);
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
    log("device", LOG_WARNING, "no device with type: {} founded ", (int)end_type::get_stype());
    return static_cast<end_type *>(nullptr);
}

template general_keyboard *find_device();
template interrupt_timer *find_device();
template general_mouse *find_device();
template generic_io_device *find_device();
template debug_device *find_device();
template generic_framebuffer *find_device();
