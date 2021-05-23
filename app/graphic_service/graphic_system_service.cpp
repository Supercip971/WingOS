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

void loop();

size_t last_key_press;
sys::server graphic_service_server;
int main(int argc, char **argv)
{
    graphic_service_server = sys::server("graphic_service.ipc");
    printf("started graphics \n");

    loop();
    return 1;
}

graphic_request_return interpret(gui::graphic_system_service_protocol *request, pid_t pid)
{
    if (request->request_type == 0)
    {
        printf("graphic error : request null type");
        return {false, 0};
    }
    else if (request->request_type == gui::GRAPHIC_SYSTEM_REQUEST::CREATE_WINDOW)
    {
        return {true, create_window(request, pid)};
    }
    else if (request->request_type == gui::GRAPHIC_SYSTEM_REQUEST::GET_WINDOW_BACK_BUFFER)
    {
        return {true, get_window_back_buffer(request, pid)};
    }
    else if (request->request_type == gui::GRAPHIC_SYSTEM_REQUEST::SWAP_WINDOW_BUFFER)
    {
        return {true, window_swap_buffer(request, pid)};
    }
    else if (request->request_type == gui::GRAPHIC_SYSTEM_REQUEST::GET_WINDOW_POSITION)
    {
        return {true, get_window_position(request, pid)};
    }
    else if (request->request_type == gui::GRAPHIC_SYSTEM_REQUEST::SET_WINDOW_POSITION)
    {
        return {false, set_window_position(request, pid)};
    }
    else if (request->request_type == gui::GRAPHIC_SYSTEM_REQUEST::WINDOW_RESIZE)
    {
        return {true, resize_window(request, pid)};
    }
    else if (request->request_type == gui::GRAPHIC_SYSTEM_REQUEST::WINDOW_DEPTH_ACTION)
    {
        if (request->depth_request.set)
        {
            return {false, window_depth_action(request, pid)};
        }
        else
        {
            return {true, window_depth_action(request, pid)};
        }
    }
    printf("graphic error : request non implemented type");
    return {false, 0};
}

void update_windows()
{
    raw_window_data *d = get_top_window();

    while (sys::get_current_keyboard_offset() > last_key_press)
    {
        auto press = sys::get_key_press(last_key_press);
        if (press.state)
        {
            gui::graphic_system_update_info info;
            info.info_type = gui::GRAPHIC_SYSTEM_INFO_SEND::KEY_INPUT;
            info.info_window_key_input.key_char = press.button;
            graphic_service_server.send(d->pid, &info, sizeof(gui::graphic_system_update_info));
        }

        last_key_press++;
    }
}

void send_focus_changed(raw_window_data *dat, bool focus_state)
{

    gui::graphic_system_update_info info;
    info.info_type = gui::GRAPHIC_SYSTEM_INFO_SEND::WINDOW_ATTRIBUTE_CHANGED;
    info.info_window_attribute_changed_info.focused = focus_state;
    info.info_window_attribute_changed_info.new_width = dat->width;
    info.info_window_attribute_changed_info.new_height = dat->height;
    info.info_window_attribute_changed_info.new_x = dat->px;
    info.info_window_attribute_changed_info.new_y = dat->py;

    graphic_service_server.send(dat->pid, &info, sizeof(gui::graphic_system_update_info));
}

void update_window_focus(uint32_t from, uint32_t to)
{
    send_focus_changed(get_window(from), false);
    send_focus_changed(get_window(to), true);
}

void loop()
{
    last_key_press = sys::get_current_keyboard_offset();

    init_cursor();
    init_raw_windows();
    init_raw_graphic_system();
    while (true)
    {
        graphic_service_server.accept_new_connection();
        update_windows();

        // read all message
        for (size_t i = 0; i < graphic_service_server.get_connection_list().size(); i++)
        {
            gui::graphic_system_service_protocol val;
            if (graphic_service_server.receive(graphic_service_server.get_connection_list()[i], &val, sizeof(gui::graphic_system_service_protocol)) == sizeof(gui::graphic_system_service_protocol))
            {
                graphic_request_return ret = interpret(&val, graphic_service_server.get_connection_list()[i]);

                if (ret.should_return)
                {
                    gui::graphic_system_return_info res;
                    res.checksum = gui::GRAPHIC_SYSTEM_INFO_SEND::RETURN_RESULT;
                    res.return_val = ret.raw_val;
                    graphic_service_server.send(graphic_service_server.get_connection_list()[i], &res, sizeof(gui::graphic_system_return_info));
                }
            }
        }
        graphic_system_update();
    }
}
