#pragma once
#include <stdint.h>

namespace sys
{
    int32_t get_mouse_x();
    int32_t get_mouse_y();
    bool get_mouse_button(int button_id);

    struct raw_request_data
    {

        uint8_t raw_data[32];
    };
    struct mouse_get_position
    {
        bool get_x_value;
    };
    struct mouse_get_button
    {
        int mouse_button_type;
    };
    struct ps2_device_request
    {
        uint8_t device_target; // for the moment 1 = mouse 2 = keyboard
        uint64_t request_type;

        union
        {
            raw_request_data data;
            mouse_get_position mouse_request_pos;
            mouse_get_button mouse_button_request;
        };
    } __attribute__((packed));
    enum mouse_request_type
    {
        GET_MOUSE_POSITION = 0,
        GET_MOUSE_BUTTON = 1
    };

    enum mouse_button_type
    {
        GET_MOUSE_LEFT_CLICK = 0,
        GET_MOUSE_RIGHT_CLICK = 1,
        GET_MOUSE_MIDDLE_CLICK = 2
    };
} // namespace sys
