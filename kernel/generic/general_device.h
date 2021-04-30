#ifndef GENERAL_DEVICE_H
#define GENERAL_DEVICE_H
#include <module/device_generic_driver.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <utils/device_file_info.h>
#include <utils/wvector.h>
#define MAX_DEVICE 128
extern const char *device_type_to_str[];

class general_device;

void add_device(general_device *dev);

general_device *get_device(uint32_t id);
uint32_t get_device_count();

template <class end_type>
extern auto find_device() -> end_type *;

#endif // TIMER_DEVICE_H
