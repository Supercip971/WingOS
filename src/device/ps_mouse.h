#pragma once
#include <stdint.h>

class ps_mouse
{

    void wait(bool is_signal);
    int32_t mouse_x_offset = 0;
    int32_t mouse_y_offset = 0;
    int32_t mouse_x = 0;
    int32_t mouse_y = 0;
    uint8_t mouse_cycle = 0;
    char mouse_data[3];

public:
    ps_mouse();

    void write(uint8_t data);
    uint8_t read();
    void init();
    void interrupt_handler();
    void set_sample_rate(uint32_t rate);
    void set_resolution(uint16_t resolution);
    int32_t get_mouse_y();
    int32_t get_mouse_x();
    static ps_mouse *the();
};
