#pragma once
#include <general_device.h>
#include <stdint.h>

class ps_mouse : public general_mouse
{

    void wait(bool is_signal);
    int32_t mouse_x_offset = 0;
    int32_t mouse_y_offset = 0;
    int32_t mouse_x = 0;
    int32_t mouse_y = 0;
    uint32_t *ptr_to_update_x = nullptr;
    uint32_t *ptr_to_update_y = nullptr;
    uint8_t mouse_cycle = 0;
    char mouse_data[3];
    bool mouse_middle = false;
    bool mouse_left = false;
    bool mouse_right = false;

public:
    ps_mouse();

    void write(uint8_t data);
    uint8_t read();
    void init();
    void update_packets();
    void interrupt_handler(unsigned int irq);
    void set_sample_rate(uint32_t rate);
    void set_resolution(uint16_t resolution);
    void set_ptr_to_update(uint32_t *x, uint32_t *y);

    uint64_t get_mouse_button(int code) final; // mouse left = 0, right = 1, middle = 2
    int32_t get_mouse_y() final;
    int32_t get_mouse_x() final;
    const char *get_name() const
    {
        return "ps2 mouse";
    }
};
