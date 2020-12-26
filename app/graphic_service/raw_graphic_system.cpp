#include "raw_graphic_system.h"
#include "cursor.h"
#include <klib/graphic_system.h>
#include <klib/kernel_util.h>
#include <klib/mem_util.h>
#include <klib/mouse.h>
#include <klib/process_message.h>
#include <klib/raw_graphic.h>
#include <klib/syscall.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

sys::pixel *front_buffer;
sys::pixel *back_buffer;
uint64_t real_gbuffer_addr = 0x0;
uint64_t screen_width = 0;
uint64_t screen_height = 0;

uint64_t get_scr_width()
{
    return screen_width;
}
uint64_t get_scr_height()
{
    return screen_height;
}

void draw_window(raw_window_data window)
{
    const uint64_t win_width = window.width;
    const uint64_t win_height = window.height;
    for (uint64_t x = 0; x < win_width; x++)
    {
        for (uint64_t y = 0; y < win_height; y++)
        {
            const uint64_t pos_f = (x + window.px) + (y + window.py) * screen_width;
            const uint64_t pos_t = (x) + (y)*win_width;
            if (pos_f > screen_width * screen_height)
            {
                return;
            }
            if (window.window_front_buffer[pos_t].a == 0)
            {
                continue;
            }
            back_buffer[pos_f].pix = window.window_front_buffer[pos_t].pix;
        }
    }
}
///TODO: move that to cursor.h/.cpp
uint64_t m_width = 7;
uint64_t m_height = 8;
const char main_cursor_mouse_buffer[] = {
    1, 0, 0, 0, 0, 0, 0,
    1, 1, 0, 0, 0, 0, 0,
    1, 2, 1, 0, 0, 0, 0,
    1, 2, 2, 1, 0, 0, 0,
    1, 2, 2, 2, 1, 0, 0,
    1, 2, 2, 2, 2, 1, 0,
    1, 2, 2, 2, 2, 2, 1,
    1, 1, 1, 1, 1, 1, 1};
void draw_mouse(uint64_t x, uint64_t y)
{
    if (x >= (screen_width - (m_width + 1)))
    {
        x = screen_width - (m_width + 1);
    }
    if (y >= screen_height - (m_height + 1))
    {
        y = screen_height - (m_height + 1);
    }
    for (uint64_t ix = 0; ix < m_width; ix++)
    {
        for (uint64_t iy = 0; iy < m_height; iy++)
        {
            const uint64_t m_index = ix + iy * m_width;
            const uint64_t m_col = main_cursor_mouse_buffer[m_index];
            if (m_col == 0)
            {
                continue;
            }
            const uint64_t m_toscreen_idx = (ix + x) + (iy + y) * screen_width;

            if (m_col == 1)
            {
                back_buffer[m_toscreen_idx] = sys::pixel(255, 255, 255);
            }
            else
            {
                back_buffer[m_toscreen_idx] = sys::pixel(0, 0, 0);
            }
        }
    }
}

void init_raw_graphic_system()
{

    printf("init raw graphic system \n");
    real_gbuffer_addr = sys::get_graphic_buffer_addr();
    front_buffer = (sys::pixel *)real_gbuffer_addr;
    screen_width = sys::get_screen_width();
    printf("screen width = %x \n", screen_width);
    screen_height = sys::get_screen_height();
    printf("screen width = %x \n", screen_height);
    printf("init raw graphic system %x x %x \n", screen_width, screen_height);
    back_buffer = new sys::pixel[(screen_width + 2) * (screen_height + 2) + 32];
}
void graphic_system_update()
{
    update_mouse_in_window();

    draw_all_window();
    update_mouse();

    swap_buffer(front_buffer, back_buffer, screen_width * screen_height);
    sys::switch_process();
}
