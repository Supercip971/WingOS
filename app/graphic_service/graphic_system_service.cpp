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
// [!] BEFORE READING THIS CODE
// [!] EVERYTHING HERE WILL BE DIVIDED IN MULTIPLE FILE FOR THE MOMENT IT IS LIKE THAT
// [!] I WILL CLEAN UP EVERYTHING WHEN I WILL BE ABLE JUST TO CLEAR A WINDOW FROM AN APPLICATION
void loop();
int main()
{
    printf("started graphics \n");
    loop();
    return 1;
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

void loop()
{

    init_cursor();
    init_raw_windows();
    init_raw_graphic_system();
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
        graphic_system_update();
    }
}
