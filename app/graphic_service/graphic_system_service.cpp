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
#define MAX_WINDOW 255

// [!] BEFORE READING THIS CODE
// [!] EVERYTHING HERE WILL BE DIVIDED IN MULTIPLE FILE FOR THE MOMENT IT IS LIKE THAT
// [!] I WILL CLEAN UP EVERYTHING WHEN I WILL BE ABLE JUST TO CLEAR A WINDOW FROM AN APPLICATION
void loop();
int main()
{
    printf("started the wingos graphic system service \n");
    loop();
    return 1;
}

struct raw_window_data
{
    uint64_t wid;
    uint64_t pid;
    uint64_t px;
    uint64_t py;
    uint64_t width;
    uint64_t height;
    char *window_name;
    sys::pixel *window_front_buffer;
    sys::pixel *window_back_buffer;
    bool used;
    bool background;

    bool should_redraw;
    uint64_t depth;
};
uint64_t *mouse_on_window;
sys::pixel *front_buffer;
sys::pixel *back_buffer;
uint64_t real_gbuffer_addr = 0x0;
uint64_t screen_width = 0;
uint64_t screen_height = 0;
uint64_t window_count = 0;
raw_window_data *window_list;

uint64_t last_window_x = 10;
uint64_t last_window_y = 10;
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
void draw_window(raw_window_data window, sys::pixel *buffer)
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
            buffer[pos_f].pix = window.window_front_buffer[pos_t].pix;
        }
    }
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

uint64_t can_use_window(uint64_t target_wid, uint64_t pid)
{
    if (target_wid > MAX_WINDOW)
    {
        printf("graphic : trying to get a window that doesn't exist (wid > MAX_WINDOW)");
        return -2;
    }
    else if (window_list[target_wid].used == false)
    {
        return -1;
        printf("not valid window id, the window is doesn't exist ");
    }
    else if (window_list[target_wid].pid != pid)
    {
        printf("the current window isn't from the current process");
        return 0;
    }
    return 1;
}
uint64_t get_window_back_buffer(sys::graphic_system_service_protocol *request, uint64_t pid)
{
    if (!can_use_window(request->get_request.window_handler_code, pid))
    {
        return -2;
    }
    return (uint64_t)window_list[request->get_request.window_handler_code].window_back_buffer;
}
uint64_t window_swap_buffer(sys::graphic_system_service_protocol *request, uint64_t pid)
{
    if (!can_use_window(request->get_request.window_handler_code, pid))
    {
        return -2;
    }
    raw_window_data &target = window_list[request->get_request.window_handler_code];
    swap_buffer(target.window_front_buffer, target.window_back_buffer, target.width * target.height);
    return 1;
}
uint64_t get_window_position(sys::graphic_system_service_protocol *request, uint64_t pid)
{
    if (!can_use_window(request->get_request.window_handler_code, pid))
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
    if (!can_use_window(request->set_pos.window_handler_code, pid))
    {
        return -2;
    }
    raw_window_data &target = window_list[request->get_request.window_handler_code];

    target.px = request->set_pos.position.rpos.x;
    target.py = request->set_pos.position.rpos.y;
    if ((target.px + target.width) > screen_width)
    {
        target.px = screen_width - target.width;
    }
    if ((target.py + target.height) > screen_height)
    {
        target.py = screen_height - target.height;
    }
    return 1;
}

uint64_t window_depth_action(sys::graphic_system_service_protocol *request, uint64_t pid)
{
    if (request->depth_request.set)
    {
        if (!can_use_window(request->depth_request.window_handler_code, pid))
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
uint64_t interpret(sys::graphic_system_service_protocol *request, uint64_t pid)
{
    if (request->request_type == 0)
    {
        printf("graphic error : request null type");
        return -2;
    }
    else if (request->request_type == sys::GRAPHIC_SYSTEM_REQUEST::CREATE_WINDOW)
    {
        return create_window(request, pid);
    }
    else if (request->request_type == sys::GRAPHIC_SYSTEM_REQUEST::GET_WINDOW_BACK_BUFFER)
    {
        return get_window_back_buffer(request, pid);
    }
    else if (request->request_type == sys::GRAPHIC_SYSTEM_REQUEST::SWAP_WINDOW_BUFFER)
    {
        return window_swap_buffer(request, pid);
    }
    else if (request->request_type == sys::GRAPHIC_SYSTEM_REQUEST::GET_WINDOW_POSITION)
    {
        return get_window_position(request, pid);
    }
    else if (request->request_type == sys::GRAPHIC_SYSTEM_REQUEST::SET_WINDOW_POSITION)
    {
        return set_window_position(request, pid);
    }
    else if (request->request_type == sys::GRAPHIC_SYSTEM_REQUEST::WINDOW_DEPTH_ACTION)
    {
        return window_depth_action(request, pid);
    }
    printf("graphic error : request non implemented type");
    return -2;
}

uint64_t m_width = 7;
uint64_t m_height = 8;
char main_cursor_mouse_buffer[] = {
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
int32_t m_x = 0;
int32_t m_y = 0;
sys::process_message mouse_msg_x;
sys::process_message mouse_msg_y;

sys::ps2_device_request mouse_requestx = {0};
sys::ps2_device_request mouse_requesty = {0};
void update_mouse()
{

    m_x = sys::get_mouse_x();
    m_y = sys::get_mouse_y();
}

bool is_mouse_in_window(uint64_t wid)
{
    if (window_list[wid].px <= m_x && window_list[wid].width + window_list[wid].px >= m_x)
    {
        if (window_list[wid].py <= m_y && window_list[wid].height + window_list[wid].py >= m_y)
        {
            return true;
        }
    }
    return false;
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

                draw_window(window_list[i], back_buffer);
                break;
            }
        }
        current_window--;
    }
}
int32_t last_mx = 0;
int32_t last_my = 0;
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
                *mouse_on_window = window_list[i].wid;
                least_depth = window_list[i].depth;
                if (least_depth == 0)
                {

                    break;
                }
            }
        }
    }
}
void loop()
{
    window_count = 0;

    real_gbuffer_addr = sys::get_graphic_buffer_addr();
    front_buffer = (sys::pixel *)real_gbuffer_addr;
    screen_width = sys::get_screen_width();
    screen_height = sys::get_screen_height();
    back_buffer = new sys::pixel[(screen_width + 1) * (screen_height + 1) + 32];
    window_list = new raw_window_data[MAX_WINDOW + 3];
    mouse_on_window = (uint64_t *)sys::sys$get_current_process_global_data(0, 8);
    *mouse_on_window = 0;
    for (int i = 0; i < MAX_WINDOW; i++)
    {

        window_list[i].used = false;
    }
    while (true)
    {
        // read all message
        while (true)
        {
            sys::raw_process_message *msg = sys::service_read_current_queue();
            if (msg != 0x0)
            {
                sys::graphic_system_service_protocol *pr = (sys::graphic_system_service_protocol *)msg->content_address;

                msg->response = interpret(pr, msg->from_pid);
                msg->has_been_readed = true;
            }
            else
            {

                break;
            }
        }
        update_mouse_in_window();

        draw_all_window();
        update_mouse();
        draw_mouse(m_x, m_y);

        swap_buffer(front_buffer, back_buffer, screen_width * screen_height);
        sys::switch_process();
    }
}
