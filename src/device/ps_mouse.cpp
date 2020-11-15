#include <arch/arch.h>
#include <arch/interrupt.h>
#include <device/ps_device.h>
#include <device/ps_mouse.h>
#include <logging.h>
ps_mouse main_ps_mouse;

ps_mouse::ps_mouse()
{
}

void ps_mouse::write(uint8_t data)
{
    wait(true);
    outb(0x64, 0xd4);
    wait(true);
    outb(0x60, data);
}

void ps_mouse::wait(bool is_signal)
{
    uint64_t time_out = 0xffffff;
    if (is_signal == false)
    {
        while (time_out--)
        {
            if ((inb(0x64) & 1) == 1)
            {
                return;
            }
            asm("pause");
        }
        log("ps2 mouse", LOG_ERROR) << "ps2 mouse timed out";
        return;
    }

    while (time_out--)
    {
        if ((inb(0x64) & 2) == 0)
        {
            return;
        }

        asm("pause");
    }
    log("ps2 mouse", LOG_ERROR) << "ps2 mouse timed out";
}

void ps_mouse::set_sample_rate(uint32_t rate)
{
    write(0xf3);
    read();
    write(rate);
    read();
}

void ps_mouse::set_resolution(uint16_t resolution)
{

    write(0xe8);
    read();
    write(resolution);
    read();
}

ps_mouse *ps_mouse::the()
{
    return &main_ps_mouse;
}

uint8_t ps_mouse::read()
{
    wait(false);
    return inb(0x60);
}

void irq_handle_mouse(unsigned int handler)
{
    lock(&ps_lock);
    ps_mouse::the()->interrupt_handler(handler);
    unlock(&ps_lock);
}

void ps_mouse::init()
{
    mouse_x = 0;
    mouse_x_offset = 0;
    mouse_y = 0;
    mouse_y_offset = 0;
    add_irq_handler(irq_handle_mouse, 12);
    log("ps2 mouse", LOG_DEBUG) << "loading ps2 mouse";

    log("ps2 mouse", LOG_INFO) << "turning on mouse";
    wait(true);
    outb(0x64, 0xA8);

    log("ps2 mouse", LOG_INFO) << "turning on mouse interrupt";

    wait(true);
    outb(0x64, 0x20);

    wait(false);
    uint8_t current_setting = inb(0x60) | 2;

    wait(true);
    outb(0x64, 0x60);

    wait(true);
    outb(0x60, current_setting);

    log("ps2 mouse", LOG_INFO) << "reset mouse settings ";

    // reset mouse to default
    write(0xF6);
    read();

    log("ps2 mouse", LOG_INFO) << "set mouse sample rate";
    // set sample rate
    set_sample_rate(20);

    log("ps2 mouse", LOG_INFO) << "set mouse resolution";
    // set resolution
    set_resolution(3);

    log("ps2 mouse", LOG_INFO) << "enable mouse data receiving";
    // enable data receiving
    write(0xF4);
    read();
}

void ps_mouse::interrupt_handler(unsigned int irq)
{

    if (mouse_cycle == 0)
    {

        mouse_cycle++;
        mouse_data[0] = inb(0x60);
    }
    else if (mouse_cycle == 1)
    {

        mouse_cycle++;
        mouse_data[1] = inb(0x60);
    }
    else
    {

        mouse_data[2] = inb(0x60);

        mouse_x_offset = mouse_data[1];
        mouse_y_offset = mouse_data[2];
        mouse_x += mouse_x_offset;

        if (mouse_x < 0)
        {
            mouse_x = 0;
        }

        mouse_y -= mouse_y_offset;
        if (mouse_y < 0)
        {
            mouse_y = 0;
        }

        mouse_middle = (bool)((mouse_data[0] >> 2) & 1);
        mouse_right = (bool)((mouse_data[0] >> 1) & 1);
        mouse_left = (bool)((mouse_data[0]) & 1);
        mouse_cycle = 0;

        if (ptr_to_update_x != nullptr)
        {
            *ptr_to_update_x = mouse_x;
        }

        if (ptr_to_update_y != nullptr)
        {
            *ptr_to_update_y = mouse_y;
        }
    }
}

void ps_mouse::set_ptr_to_update(uint32_t *x, uint32_t *y)
{
    ptr_to_update_x = x;
    ptr_to_update_y = y;
}

int32_t ps_mouse::get_mouse_x()
{
    return mouse_x;
}

int32_t ps_mouse::get_mouse_y()
{
    return mouse_y;
}

uint64_t ps_mouse::get_mouse_button(int code)
{
    if (code == 0)
    {
        return mouse_left;
    }
    else if (code == 1)
    {
        return mouse_right;
    }

    return mouse_middle;
}
