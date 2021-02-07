#include <gui/graphic_system.h>
#include <gui/window.h>
#include <kern/kernel_util.h>
#include <kern/mem_util.h>
#include <kern/process_message.h>
#include <kern/syscall.h>
//#include <feather_language_lib/feather.h>
#include <gui/raw_graphic.h>
#include <gui/widget/button_widget.h>
#include <gui/widget/rectangle_widget.h>
#include <gui/widget/terminal_widget.h>
#include <kern/process_buffer.h>
#include <stdio.h>
#include <stdlib.h>
void click(uint64_t t)
{
    printf("%x \n", t);
}
int main(int argc, char **argv)
{
    auto pid = sys::start_programm("init_fs/shell.exe");
    gui::window test_window("termix", 600, 400);
    gui::terminal_widget *button = new gui::terminal_widget(10, 20, 600 - 20, 400 - 30, pid);

    test_window.add_widget((gui::widget *)button);

    test_window.start();
    while (true)
    {
    }
}
