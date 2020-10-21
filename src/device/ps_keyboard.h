#pragma once
#include <stdint.h>
#define KEY_COUNT 128
class ps_keyboard
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
    bool key_pressed[KEY_COUNT];
    uint8_t *ptr_to_update;

public:
    ps_keyboard();

    void write(uint8_t data);
    uint8_t read();
    void init();
    void interrupt_handler();
    void set_key(bool state, uint8_t keycode);
    bool get_key(uint8_t keycode);
    void set_ptr_to_update(uint32_t *d);
    uint64_t get_mouse_button(int code); // mouse left = 0, right = 1, middle = 2
    int32_t get_mouse_y();
    int32_t get_mouse_x();
    static ps_keyboard *the();
};
