#include <gui/graphic_system.h>
#include <gui/window.h>
#include <kern/kernel_util.h>
#include <kern/mem_util.h>
#include <kern/process_message.h>
#include <kern/syscall.h>
//#include <feather_language_lib/feather.h>
#include <gui/raw_graphic.h>
#include <gui/widget/button.h>
#include <gui/widget/rectangle.h>
#include <gui/widget/terminal.h>
#include <kern/process_buffer.h>
#include <stdio.h>
#include <stdlib.h>
void click(uint64_t t)
{
    printf("%x \n", t);
}
int main(int argc, char **argv)
{
    auto pid = sys::start_programm("initfs/shell.exe");
    if (pid == 0)
    {
        printf("unable to start shell programm ");
        while (true)
        {
        }
    }
    gui::window test_window("termix", 600, 400);
    gui::terminal_widget *button = new gui::terminal_widget(10, 20, 600 - 20, 400 - 30, pid);

    test_window.add_widget((gui::widget *)button);

    test_window.start();
    while (true)
    {
    }
}
