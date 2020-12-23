#include "cursor.h"
#include "raw_graphic_system.h"
#include "raw_window.h"
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

int32_t m_x = 0;
int32_t m_y = 0;
sys::process_message mouse_msg_x;
sys::process_message mouse_msg_y;

sys::ps2_device_request mouse_requestx = {0};
sys::ps2_device_request mouse_requesty = {0};

int32_t last_mx = 0;
int32_t last_my = 0;
uint64_t *mouse_on_window;
void init_cursor()
{
    mouse_on_window = (uint64_t *)sys::sys$get_current_process_global_data(0, 8);
    *mouse_on_window = 0;
}
bool is_mouse_in_window(uint64_t wid)
{
    raw_window_data *window_list = get_window_list();
    if (window_list[wid].px <= m_x && window_list[wid].width + window_list[wid].px >= m_x)
    {
        if (window_list[wid].py <= m_y && window_list[wid].height + window_list[wid].py >= m_y)
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

void set_mouse_on_window(uint64_t wid)
{
    *mouse_on_window = wid;
}
