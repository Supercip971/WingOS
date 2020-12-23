#include <arch/arch.h>
#include <arch/interrupt.h>
#include <device/ps_device.h>
#include <device/ps_keyboard.h>
#include <logging.h>

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
        28,
        0,
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
const char asciiShift[] =
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
        0,
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

ps_keyboard main_ps_key;

ps_keyboard::ps_keyboard()
{
}
void ps_keyboard_interrupt_handler(unsigned int irq)
{
    lock(&ps_lock);
    ps_keyboard::the()->interrupt_handler();
    unlock(&ps_lock);
}
void ps_keyboard::set_key(bool state, uint8_t keycode)
{
    key_pressed[keycode] = state;

    if (ptr_to_update != nullptr)
    {
        ptr_to_update[keycode] = state;
    }
}

bool ps_keyboard::get_key(uint8_t keycode)
{
    return key_pressed[keycode];
}

void ps_keyboard::init()
{
    for (int i = 0; i < KEY_COUNT; i++)
    {
        key_pressed[i] = false;
    }
    add_irq_handler(ps_keyboard_interrupt_handler, 1);
}

void ps_keyboard::interrupt_handler()
{
    uint8_t state = inb(0x64);
    if (state & 1)
    {
        uint8_t key_code = inb(0x60);
        uint8_t scan_code = key_code & 0x7f;
        uint8_t key_state = !(key_code & 0x80);
        set_key(key_state, scan_code);
        if (key_state)
        {
            log("keyboard", LOG_INFO) << asciiDefault[scan_code];
        }
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

ps_keyboard *ps_keyboard::the()
{
    return &main_ps_key;
}
