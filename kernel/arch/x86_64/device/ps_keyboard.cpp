#include <arch.h>
#include <device/ps_device.h>
#include <device/ps_keyboard.h>
#include <interrupt.h>
#include <logging.h>

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
    add_device(this);
    for (int i = 0; i < KEY_COUNT; i++)
    {
        key_pressed[i] = false;
    }
    add_irq_handler(ps_keyboard_interrupt_handler, 1);
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
        if (key_state)
        {
            log("keyboard", LOG_INFO) << (uint64_t)asciiDefault[scan_code];
        }
        state = inb(0x64);
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
