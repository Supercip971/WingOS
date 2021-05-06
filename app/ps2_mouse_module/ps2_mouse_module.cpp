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
    char mouse_data[3] = {0};
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

ps_mouse *main_ps_mouse;

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
        echo_out("ps2 mouse timed out");
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
    echo_out("ps2 mouse timed out");
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

uint8_t ps_mouse::read()
{
    wait(false);
    return inb(0x60);
}

void ps_mouse::init()
{

    main_ps_mouse = this;
    mouse_x = 0;
    mouse_x_offset = 0;
    mouse_y = 0;
    mouse_y_offset = 0;
    wait(true);
    outb(0x64, 0xA8);

    wait(true);
    outb(0x64, 0x20);

    wait(false);
    uint8_t current_setting = inb(0x60) | 2;

    wait(true);
    outb(0x64, 0x60);

    wait(true);
    outb(0x60, current_setting);

    // reset mouse to default
    write(0xF6);
    read();

    // enable data receiving
    write(0xF4);
    read();
    add_irq_handler(12, irq_handle_mouse);
}
void ps_mouse::update_packets()
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
void ps_mouse::interrupt_handler(unsigned int irq)
{
    uint8_t status = inb(0x64);
    while ((status & 0x20) == 0x20 && (status & 0x1))
    {
        update_packets();
        status = inb(0x64);
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

void irq_handle_mouse(unsigned int handler)
{
    main_ps_mouse->interrupt_handler(handler);
}

int main(int argc, char **argv)
{
    echo_out("loading ps2 mouse module driver");
    // TODO: add a process_lock/unlock call to the kernel
    asm volatile("cli");
    ps_mouse *pss_mouse = new ps_mouse();
    pss_mouse->init();

    set_device_driver(device_type::MOUSE_DEVICE, pss_mouse);
    asm volatile("sti");
    return 0;
}

extern int __end_point()
{

    echo_out("finished ps2 mouse module driver");
    return 0;
}
