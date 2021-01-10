#pragma once
#include <general_device.h>
#include <stdint.h>
generic_io_device *get_io_device(uint64_t id);
void add_io_device(generic_io_device *dev);
