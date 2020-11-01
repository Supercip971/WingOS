#include <kgui/window.h>
#include <klib/graphic_system.h>
#include <klib/kernel_util.h>
#include <klib/mem_util.h>
#include <klib/process_message.h>
#include <klib/syscall.h>
//#include <feather_language_lib/feather.h>
#include <kgui/widget/button_widget.h>
#include <kgui/widget/rectangle_widget.h>
#include <klib/raw_graphic.h>
#include <stdio.h>
#include <stdlib.h>

int main()
{
    printf("g buffer addr : %x \n", sys::get_graphic_buffer_addr());
    sys::graphic_context gc(sys::get_screen_width(), sys::get_screen_height(), "background");

    gc.clear_buffer(sys::pixel(70, 70, 70, 255));
    gc.set_as_background();
    gc.set_graphic_context_position({0, 0});
    gc.swap_buffer();
    while (true)
    {
        sys::switch_process();
    }
    return 0;
}
