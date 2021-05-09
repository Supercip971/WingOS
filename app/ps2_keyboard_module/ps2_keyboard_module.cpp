#ifndef MODULE
#define MODULE
#endif
// for qtcreator dumb  autocomplete
#include <gui/graphic_system.h>
#include <gui/window.h>
#include <kern/kernel_util.h>
#include <kern/mem_util.h>
#include <kern/process_message.h>
#include <kern/syscall.h>
//#include <feather_language_lib/feather.h>
#include <gui/img_bmp.h>
#include <gui/raw_graphic.h>
#include <gui/widget/button.h>
#include <gui/widget/rectangle.h>
#include <module/module_calls.h>
#include <stdio.h>

#include <stdlib.h>
#include <string.h>
void irq_handle_mouse(unsigned int handler);

#define KEY_COUNT 128

static const char asciiDefault[58] =
    {
        0,
        0,  // ESC
        49, // 1
        50,
        51,
        52,
        53,
        54,
        55,
        56,
        57, // 9
        48, // 0
        45, // -
        61, // =
        8,
        0,
        113, // q
        119,
        101,
        114,
        116,
        121,
        117,
        105,
        111,
        112, // p
        91,  // [
        93,  // ]
        '\n',
        10, // return
        97, // a
        115,
        100,
        102,
        103,
        104,
        106,
        107,
        108, // l
        59,  // ;
        39,  // '
        96,  // `
        0,
        92,  // BACKSLASH
        122, // z
        120,
        99,
        118,
        98,
        110,
        109, // m
        44,  // ,
        46,  // .
        47,  // /
        0,
        42, // *
        0,
        32, // SPACE
};
static const char asciiShift[] =
    {
        0,
        0,  // ESC
        33, // !
        64, // @
        35, // #
        36, // $
        37, // %
        94, // ^
        38, // &
        42, // *
        40, // (
        41, // )
        95, // _
        43, // +
        0,
        0,
        81, // Q
        87,
        69,
        82,
        84,
        89,
        85,
        73,
        79,
        80,  // P
        123, // {
        125, // }
        0,
        10,
        65, // A
        83,
        68,
        70,
        71,
        72,
        74,
        75,
        76,  // L
        58,  // :
        34,  // "
        126, // ~
        0,
        124, // |
        90,  // Z
        88,
        67,
        86,
        66,
        78,
        77, // M
        60, // <
        62, // >
        63, // ?
        0,
        0,
        0,
        32, // SPACE
};

class ps_keyboard : public general_keyboard
{

    void wait(bool is_signal);
    int32_t mouse_x_offset = 0;
    int32_t mouse_y_offset = 0;
    int32_t mouse_x = 0;
    int32_t mouse_y = 0;
    uint8_t mouse_cycle = 0;
    uint8_t last_keypress = 0;
    bool key_pressed[KEY_COUNT];
    uint8_t *ptr_to_update;

public:
    ps_keyboard();

    void write(uint8_t data);
    uint8_t read();
    void init();
    void interrupt_handler();
    void set_key(bool state, uint8_t keycode);
    bool get_key(uint8_t keycode) const;
    uint8_t get_last_keypress() const
    {
        return asciiDefault[last_keypress];
    }
    void set_ptr_to_update(uint32_t *d);

    const char *get_name() const final
    {
        return "ps2 keyboard";
    };
};

ps_keyboard *main_ps_key;

ps_keyboard::ps_keyboard()
{
}
void ps_keyboard_interrupt_handler(unsigned int irq)
{
    main_ps_key->interrupt_handler();
}
void ps_keyboard::set_key(bool state, uint8_t keycode)
{
    key_pressed[keycode] = state;
    if (state == true)
    {
        last_keypress = keycode;
    }
    else
    {
        last_keypress = 0;
    }
    if (ptr_to_update != nullptr)
    {
        ptr_to_update[keycode] = state;
    }
}

bool ps_keyboard::get_key(uint8_t keycode) const
{
    return key_pressed[keycode];
}

void ps_keyboard::init()
{

    main_ps_key = this;
    for (int i = 0; i < KEY_COUNT; i++)
    {
        key_pressed[i] = false;
    }
    add_irq_handler(1, ps_keyboard_interrupt_handler);
}

void ps_keyboard::interrupt_handler()
{
    uint8_t state = inb(0x64);
    while (state & 1 && (state & 0x20) == 0)
    {

        uint8_t key_code = inb(0x60);
        uint8_t scan_code = key_code & 0x7f;
        uint8_t key_state = !(key_code & 0x80);
        set_key(key_state, scan_code);
        state = inb(0x64);
        keyboard_buff_info info;
        info.button = scan_code;
        info.state = key_state;
        buffer.push_back(info);
    }
}

void ps_keyboard::set_ptr_to_update(uint32_t *d)
{
    ptr_to_update = (uint8_t *)d;
    for (int i = 0; i < KEY_COUNT; i++)
    {
        ptr_to_update[i] = 0;
    }
}
int main(int argc, char **argv)
{
    echo_out("loading ps2 keyboard module driver");
    // TODO: add a process_lock/unlock call to the kernel
    asm volatile("cli");
    ps_keyboard *pss_kb = new ps_keyboard();
    pss_kb->init();

    set_device_driver(device_type::KEYBOARD_DEVICE, pss_kb);

    asm volatile("sti");
    return 0;
}

extern int __end_point()
{

    echo_out("finished ps2 mouse module driver");
    return 0;
}
