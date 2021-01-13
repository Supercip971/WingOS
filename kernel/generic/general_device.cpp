#include "general_device.h"
#include <logging.h>
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
auto find_device(device_type type) -> end_type *
{
    for (uint32_t i = 0; i < current_array_count; i++)
    {
        if (device_array[i]->get_type() == type)
        {
            return static_cast<end_type *>(device_array[i]);
        }
    }
    log("device", LOG_WARNING) << "no device with type " << (int)type << "founded";
    return static_cast<end_type *>(nullptr);
}

template general_mouse *find_device(device_type type);
template debug_device *find_device(device_type type);
