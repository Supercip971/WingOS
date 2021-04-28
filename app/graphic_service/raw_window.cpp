#include "raw_window.h"
#include "cursor.h"
#include "graphic_system_service.h"
#include "raw_graphic_system.h"
#include <kern/mem_util.h>
#include <kern/mouse_keyboard.h>
#include <kern/process_message.h>
#include <stdio.h>
#include <stdlib.h>
utils::vector<raw_window_data> window_list;

uint64_t last_window_x = 10;
uint64_t last_window_y = 10;
raw_window_data *get_top_window()
{
    if (window_list.size() < 1)
    {
        return nullptr;
    }
    return &window_list[window_list.size() - 1];
}
void init_raw_windows()
{
    printf("init window \n");
}
utils::vector<raw_window_data> get_window_list()
{
    return window_list;
}
uint64_t get_window_list_count()
{
    return window_list.size();
}
raw_window_data *get_window(uint64_t target_wid)
{
    for (size_t i = 0; i < window_list.size(); i++)
    {
        if (window_list[i].wid == target_wid)
        {
            return &window_list[i];
        }
    }
    return nullptr;
}
size_t get_window_id(uint64_t target_wid)
{
    for (size_t i = 0; i < window_list.size(); i++)
    {
        if (window_list[i].wid == target_wid)
        {
            return i;
        }
    }
    return -1;
}

bool valid_window(uint64_t target_wid, uint64_t pid)
{
    auto raw = get_window(target_wid);
    if (raw == nullptr)
    {
        return false;
    }
    else if (raw->pid != pid)
    {
        return false;
    }
    return true;
}
void set_window_on_top(uint64_t wid)
{
    auto raw = get_window(wid);
    size_t idx = get_window_id(wid);
    raw_window_data raw_stack = *raw;
    window_list.remove(idx);
    window_list.push_back(raw_stack);
}
void set_window_background(uint64_t wid)
{
    auto raw = get_window(wid);
    raw_window_data raw_stack = *raw;
    size_t idx = get_window_id(wid);
    window_list.remove(idx);
    window_list.push_front(raw_stack);
}
void set_window_top_background(uint64_t wid)
{
    auto raw = get_window(wid);
    raw_window_data raw_stack = *raw;
    size_t idx = get_window_id(wid);
    window_list.remove(idx);
    window_list.insert(1, raw_stack);
}
uint32_t next_wid = 10;

uint64_t create_window(gui::graphic_system_service_protocol *request, uint64_t pid)
{
    raw_window_data targ = {0};

    targ.pid = pid;
    targ.width = request->create_window_info.width;
    targ.height = request->create_window_info.height;
    targ.px = last_window_x;
    last_window_x += 10;
    if (last_window_x > 100)
    {
        last_window_y += 10;
        last_window_x = 10;
    }
    last_window_y += 10;
    targ.should_redraw = true;
    targ.py = last_window_y;
    targ.window_name = request->create_window_info.name;
    targ.wid = next_wid++;
    targ.window_front_buffer = (gui::color *)sys::pmm_malloc_shared(request->create_window_info.width * request->create_window_info.height * sizeof(gui::color));
    targ.window_back_buffer = (gui::color *)sys::pmm_malloc_shared((request->create_window_info.width + 64) * (request->create_window_info.height + 64) * sizeof(gui::color));
    window_list.push_back(targ);
    set_window_on_top(targ.wid);

    return targ.wid;
}

uint64_t get_window_back_buffer(gui::graphic_system_service_protocol *request, uint64_t pid)
{
    if (!valid_window(request->get_request.window_handler_code, pid))
    {
        printf("invalid get window back buffer call \n");
        return -2;
    }
    auto raw = get_window(request->get_request.window_handler_code);
    return (uint64_t)raw->window_back_buffer;
}

uint64_t window_swap_buffer(gui::graphic_system_service_protocol *request, uint64_t pid)
{
    if (!valid_window(request->get_request.window_handler_code, pid))
    {
        printf("invalid window swap buffer buffer call \n");
        return -2;
    }

    auto raw = get_window(request->get_request.window_handler_code);
    swap_buffers(raw->window_front_buffer, raw->window_back_buffer, raw->width * raw->height);
    return 1;
}
uint64_t get_window_position(gui::graphic_system_service_protocol *request, uint64_t pid)
{
    if (!valid_window(request->get_request.window_handler_code, pid))
    {
        printf("invalid get window position call \n");
        return -2;
    }
    auto raw = get_window(request->get_request.window_handler_code);
    sys::raw_pos pos = {0};

    pos.rpos.x = raw->px;
    pos.rpos.y = raw->py;

    return (uint64_t)pos.pos;
}
uint64_t set_window_position(gui::graphic_system_service_protocol *request, uint64_t pid)
{
    if (!valid_window(request->set_pos.window_handler_code, pid))
    {
        printf("invalid set window position call \n");
        return -2;
    }
    auto raw = get_window(request->set_pos.window_handler_code);

    raw->px = request->set_pos.position.rpos.x;
    raw->py = request->set_pos.position.rpos.y;
    if ((raw->px + raw->width) > get_scr_width())
    {
        raw->px = get_scr_width() - raw->width;
    }
    if ((raw->py + raw->height) > get_scr_height())
    {
        raw->py = get_scr_height() - raw->height;
    }
    return 1;
}

uint64_t window_depth_action(gui::graphic_system_service_protocol *request, uint64_t pid)
{
    if (request->depth_request.set)
    {
        if (!valid_window(request->depth_request.window_handler_code, pid))
        {
            printf("havn't the right to use the window\n");
            return -2;
        }
        if (request->depth_request.type == gui::ON_TOP)
        {
            set_window_on_top(request->depth_request.window_handler_code);
            return 1;
        }
        else if (request->depth_request.type == gui::TOP_BACKGROUND)
        {
            set_window_top_background(request->depth_request.window_handler_code);
            return 1;
        }
        else if (request->depth_request.type == gui::BACKGROUND)
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

        if (!valid_window(request->depth_request.window_handler_code, pid))
        {
            return -2;
        }

        auto raw = get_window_id(request->depth_request.window_handler_code);
        return window_list.size() - raw - 1;
    }
}

void draw_all_window()
{
    for (int i = 0; i < window_list.size(); i++)
    {

        draw_window(window_list[i]);
    }
}

bool start_click = false;
void update_mouse_in_window()
{
    for (int i = window_list.size(); i > 0; i--)
    {
        if (is_mouse_in_window(&window_list[i]))
        {

            set_mouse_on_window(&window_list[i]);
            break;
        }
    }
    if (sys::get_mouse_button(sys::mouse_button_type::GET_MOUSE_LEFT_CLICK) && window_list.size() >= 2)
    {
        if (!start_click)
        {

            start_click = true;
            auto window = get_window(get_mouse_on_window_wid());
            // FIXME: add a get_top_window()
            if (!window->background && window->wid != window_list[window_list.size() - 1].wid)
            {
                uint32_t previous = window_list[window_list.size() - 1].wid;
                uint32_t next = window->wid;
                set_window_on_top(next);
                update_window_focus(previous, next);
            }
        }
    }
    else
    {
        start_click = false;
    }
}
