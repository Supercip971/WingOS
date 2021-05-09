#include "cursor.h"
#include "raw_graphic_system.h"
#include "raw_window.h"
#include <gui/graphic_system.h>
#include <gui/raw_graphic.h>
#include <kern/kernel_util.h>
#include <kern/mem_util.h>
#include <kern/mouse_keyboard.h>
#include <kern/process_message.h>
#include <kern/syscall.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

int32_t m_x = 0;
int32_t m_y = 0;

sys::ps2_device_request mouse_requestx = {0};
sys::ps2_device_request mouse_requesty = {0};

int32_t last_mx = 0;
int32_t last_my = 0;
uint64_t mouse_on_window;
void init_cursor()
{
    printf("init cursor \n");
    mouse_on_window = 0;
}

uint64_t get_mouse_on_window_wid()
{
    return mouse_on_window;
}

bool is_mouse_in_window(raw_window_data *window)
{
    if (window->px <= m_x && window->width + window->px >= m_x)
    {
        if (window->py <= m_y && window->height + window->py >= m_y)
        {
            return true;
        }
    }
    return false;
}

void update_mouse()
{

    m_x = sys::get_mouse_x();
    m_y = sys::get_mouse_y();

    draw_mouse(m_x, m_y);
}

void set_mouse_on_window(raw_window_data *window)
{
    mouse_on_window = window->wid;
}
