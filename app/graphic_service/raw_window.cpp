#include "raw_window.h"
#include "cursor.h"
#include "raw_graphic_system.h"
#include <stdio.h>
#include <stdlib.h>
uint64_t window_count = 0;
raw_window_data *window_list;

uint64_t last_window_x = 10;
uint64_t last_window_y = 10;

void init_raw_windows()
{

    window_count = 0;
    window_list = new raw_window_data[MAX_WINDOW + 3];

    for (int i = 0; i < MAX_WINDOW; i++)
    {
        window_list[i].used = false;
    }
}
raw_window_data *get_window_list()
{
    return window_list;
}
uint64_t get_window_list_count()
{
    return window_count;
}
uint64_t can_use_window(uint64_t target_wid, uint64_t pid)
{
    if (target_wid > MAX_WINDOW)
    {
        return -2;
    }
    else if (window_list[target_wid].used == false)
    {
        return -1;
    }
    else if (window_list[target_wid].pid != pid)
    {
        return 0;
    }
    return 1;
}

bool valid_window(uint64_t target_wid, uint64_t pid)
{
    if (target_wid > MAX_WINDOW)
    {
        return false;
    }
    else if (window_list[target_wid].used == false)
    {
        return false;
    }
    else if (window_list[target_wid].pid != pid)
    {
        return false;
    }
    return true;
}
void set_window_on_top(uint64_t wid)
{
    uint64_t prev_depth = window_list[wid].depth;
    for (int i = 0; i < window_count; i++)
    {
        if (window_list[i].used == false)
        {
            continue;
        }
        if (window_list[i].wid == wid)
        {
            window_list[i].depth = 0;
        }
        else
        {
            if (window_list[i].depth <= prev_depth)
            {

                window_list[i].depth++;
            }
        }
    }
}
void set_window_background(uint64_t wid)
{
    uint64_t max_depth = 0;
    uint64_t widt = 0;
    for (int i = 0; i < window_count; i++)
    {
        if (window_list[i].used == false)
        {
            continue;
        }
        if (window_list[i].wid == wid)
        {
            widt = i;
        }
        else
        {
            if (window_list[i].depth > max_depth)
            {
                max_depth = window_list[i].depth;
            }
        }
    }

    window_list[widt].background = true;
    for (int i = 0; i < window_count; i++)
    {
        if (window_list[i].used == false)
        {
            continue;
        }
        if (window_list[widt].depth < window_list[i].depth)
        {
            window_list[i].depth--;
        }
    }
    window_list[widt].depth = max_depth;
}
void set_window_top_background(uint64_t wid)
{
    uint64_t max_depth = 0;
    uint64_t widt = 0;

    for (int i = 0; i < window_count; i++)
    {
        if (window_list[i].used == false)
        {
            continue;
        }
        if (window_list[i].wid == wid)
        {
            widt = i;
        }
        else
        {
            if (window_list[i].depth > max_depth && !window_list[i].background)
            {
                max_depth = window_list[i].depth;
            }
        }
    }
    window_list[widt].depth = max_depth;
}

uint64_t create_window(sys::graphic_system_service_protocol *request, uint64_t pid)
{
    for (int i = 0; i < MAX_WINDOW; i++)
    {
        if (window_list[i].used == false)
        {
            window_list[i].used = true;
            window_count++;
            window_list[i].pid = pid;
            window_list[i].width = request->create_window_info.width;
            window_list[i].height = request->create_window_info.height;
            window_list[i].px = last_window_x;
            last_window_x += 10;
            if (last_window_x > 100)
            {
                last_window_y += 10;
                last_window_x = 10;
            }
            last_window_y += 10;
            window_list[i].should_redraw = true;
            window_list[i].py = last_window_y;
            window_list[i].window_name = request->create_window_info.name;
            window_list[i].wid = i;
            window_list[i].window_front_buffer = (sys::pixel *)malloc(request->create_window_info.width * request->create_window_info.height * sizeof(sys::pixel));
            window_list[i].window_back_buffer = (sys::pixel *)malloc(request->create_window_info.width * request->create_window_info.height * sizeof(sys::pixel));
            window_list[i].depth = 100;
            set_window_on_top(window_list[i].wid);

            return i;
        }
    }

    return -1;
}

uint64_t get_window_back_buffer(sys::graphic_system_service_protocol *request, uint64_t pid)
{
    if (!valid_window(request->get_request.window_handler_code, pid))
    {
        return -2;
    }
    return (uint64_t)window_list[request->get_request.window_handler_code].window_back_buffer;
}

uint64_t window_swap_buffer(sys::graphic_system_service_protocol *request, uint64_t pid)
{
    if (!valid_window(request->get_request.window_handler_code, pid))
    {
        return -2;
    }
    raw_window_data &target = window_list[request->get_request.window_handler_code];
    swap_buffer(target.window_front_buffer, target.window_back_buffer, target.width * target.height);
    return 1;
}
uint64_t get_window_position(sys::graphic_system_service_protocol *request, uint64_t pid)
{
    if (!valid_window(request->get_request.window_handler_code, pid))
    {
        return -2;
    }
    raw_window_data &target = window_list[request->get_request.window_handler_code];
    sys::raw_pos pos = {0};

    pos.rpos.x = target.px;
    pos.rpos.y = target.py;

    return (uint64_t)pos.pos;
}
uint64_t set_window_position(sys::graphic_system_service_protocol *request, uint64_t pid)
{
    if (!valid_window(request->set_pos.window_handler_code, pid))
    {
        return -2;
    }
    raw_window_data &target = window_list[request->get_request.window_handler_code];

    target.px = request->set_pos.position.rpos.x;
    target.py = request->set_pos.position.rpos.y;
    if ((target.px + target.width) > get_scr_width())
    {
        target.px = get_scr_width() - target.width;
    }
    if ((target.py + target.height) > get_scr_height())
    {
        target.py = get_scr_height() - target.height;
    }
    return 1;
}

uint64_t window_depth_action(sys::graphic_system_service_protocol *request, uint64_t pid)
{
    if (request->depth_request.set)
    {
        if (!valid_window(request->depth_request.window_handler_code, pid))
        {
            printf("havn't the right to use the window\n");
            return -2;
        }
        if (request->depth_request.type == sys::ON_TOP)
        {
            set_window_on_top(request->depth_request.window_handler_code);
            return 1;
        }
        else if (request->depth_request.type == sys::TOP_BACKGROUND)
        {
            set_window_top_background(request->depth_request.window_handler_code);
            return 1;
        }
        else if (request->depth_request.type == sys::BACKGROUND)
        {
            set_window_background(request->depth_request.window_handler_code);
            return 1;
        }
        else
        {
            return -2;
        }
    }
    else
    {
        return window_list[request->depth_request.window_handler_code].depth;
    }
}

void draw_all_window()
{
    if (window_count == 0)
    {

        return;
    }
    int current_window = window_count + 2;
    while (current_window >= 0)
    {
        for (int i = 0; i < window_count; i++)
        {

            if (window_list[i].used == true && window_list[i].depth == current_window)
            {

                draw_window(window_list[i]);
                break;
            }
        }
        current_window--;
    }
}
void update_mouse_in_window()
{
    int least_depth = 10000;
    for (int i = 0; i < window_count; i++)
    {
        if (window_list[i].depth < least_depth)
        {
            bool is_in_window = is_mouse_in_window(window_list[i].wid);
            if (is_in_window)
            {
                set_mouse_on_window(window_list[i].wid);
                least_depth = window_list[i].depth;
                if (least_depth == 0)
                {

                    break;
                }
            }
        }
    }
}
