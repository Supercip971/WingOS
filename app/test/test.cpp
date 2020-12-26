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
    gui::window test_window("my window", 300, 200);
    gui::button_widget *button = new gui::button_widget(10, 30, 70, 20, "a button");

    button->set_click_callback(click);
    test_window.add_widget((gui::widget *)button);

    return test_window.start();
    return 0;
}
