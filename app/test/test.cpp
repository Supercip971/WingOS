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
#include <kern/process_buffer.h>
#include <stdio.h>
#include <stdlib.h>
void click(uint64_t t)
{
    printf("%x \n", t);
}
int main(int argc, char **argv)
{

    printf("starting test.exe \n");
    printf("hello world ! <3 \n");
    gui::window test_window("my window", 300, 200);
    gui::button_widget *button = new gui::button_widget(10, 30, 70, 20, "a button");

    button->set_click_callback(click);
    test_window.add_widget(dynamic_cast<gui::widget *>(button));

    return test_window.start();
}
