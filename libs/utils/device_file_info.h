#ifndef DEVICE_FILE_INFO_H
#define DEVICE_FILE_INFO_H
#include <stdint.h>

#define MOUSE_FILE_BUFFER "/dev/mouse"
#define KEYBOARD_FILE_BUFFER "/dev/keyboard"

struct mouse_buff_info
{
    int32_t mouse_x;
    int32_t mouse_y;
    bool left;
    bool right;
    bool middle;
} __attribute__((packed));

struct keyboard_buff_info
{
    uint16_t button;
    uint8_t state;
} __attribute__((packed));

#endif // DEVICE_FILE_INFO_H
