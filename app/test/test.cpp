#include <kgui/window.h>
#include <klib/graphic_system.h>
#include <klib/kernel_util.h>
#include <klib/mem_util.h>
#include <klib/process_message.h>
#include <klib/syscall.h>
//#include <feather_language_lib/feather.h>
#include <kgui/widget/button_widget.h>
#include <kgui/widget/rectangle_widget.h>
#include <klib/process_buffer.h>
#include <klib/raw_graphic.h>
#include <stdio.h>
#include <stdlib.h>
void click(uint64_t t)
{
    printf("%x \n", t);
}
int main()
{
    printf("g buffer addr : %x \n", sys::get_graphic_buffer_addr());
    gui::window test_window("my window", 300, 200);
    gui::button_widget button(10, 30, 70, 20, "a button");

    button.set_click_callback(click);
    test_window.add_widget((gui::widget *)&button);
    sys::process_buffer pb(sys::get_process_pid("init_fs/graphic_service.exe"), sys::process_buffer_type::STDOUT);
    int pb_lenth = pb.get_length();
    char *buffer = new char[pb_lenth + 2];
    buffer[pb_lenth + 1] = 0;
    pb.next((uint8_t *)buffer, pb_lenth);
    printf("| %s | ", buffer);
    return test_window.start();
    return 0;
}
