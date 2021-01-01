#include <kgui/window.h>
#include <klib/graphic_system.h>
#include <klib/kernel_util.h>
#include <klib/mem_util.h>
#include <klib/process_message.h>
#include <klib/syscall.h>
//#include <feather_language_lib/feather.h>
#include <ctypes.h>
#include <kgui/widget/button_widget.h>
#include <kgui/widget/rectangle_widget.h>
#include <kgui/widget/terminal_widget.h>
#include <klib/process_buffer.h>
#include <klib/raw_graphic.h>
#include <klib/shell.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
uint64_t interpret(sys::shell_protocol *protocol)
{
    if (protocol->action == sys::SHELL_SEND_COMMAND)
    {
        printf("sending command %s \n", protocol->command.command);
    }
    else if (protocol->action == sys::SHELL_SEND_KEY)
    {
    }
    return 1;
}
int main()
{
    while (true)
    {
        while (true)
        {
            sys::raw_process_message *msg = sys::service_read_current_queue();
            if (msg != 0x0)
            {
                sys::shell_protocol *pr = (sys::shell_protocol *)msg->content_address;

                msg->response = interpret(pr);
                msg->has_been_readed = true;
            }
            else
            {

                break;
            }
        }
    }
    return 0;
}
