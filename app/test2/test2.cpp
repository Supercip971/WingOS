#include <kgui/window.h>
#include <klib/graphic_system.h>
#include <klib/kernel_util.h>
#include <klib/mem_util.h>
#include <klib/process_message.h>
#include <klib/syscall.h>
//#include <feather_language_lib/feather.h>
#include <kgui/widget/button_widget.h>
#include <kgui/widget/rectangle_widget.h>
#include <kgui/widget/terminal_widget.h>
#include <klib/process_buffer.h>
#include <klib/raw_graphic.h>
#include <stdio.h>
#include <stdlib.h>
void click(uint64_t t)
{
    printf("%x \n", t);
}
int main(int argc, char **argv)
{
    /*
    gui::window test_window("termix", 600, 400);
    gui::terminal_widget *button = new gui::terminal_widget(10, 20, 600 - 20, 400 - 30, sys::get_process_pid("init_fs/test.exe"));

    test_window.add_widget((gui::widget *)button);
*/

    while (true)
    {
    }
}
