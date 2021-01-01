#pragma once
#include <stdint.h>
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

class ps_keyboard
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
    bool get_key(uint8_t keycode);
    uint8_t get_last_keypress() const
    {
        return asciiDefault[last_keypress];
    }
    void set_ptr_to_update(uint32_t *d);
    static ps_keyboard *the();
};
