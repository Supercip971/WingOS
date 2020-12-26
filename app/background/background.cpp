#include <kgui/window.h>
#include <klib/graphic_system.h>
#include <klib/kernel_util.h>
#include <klib/mem_util.h>
#include <klib/process_message.h>
#include <klib/syscall.h>
//#include <feather_language_lib/feather.h>
#include <kgui/img_bmp.h>
#include <kgui/widget/button_widget.h>
#include <kgui/widget/rectangle_widget.h>
#include <klib/raw_graphic.h>
#include <stdio.h>
#include <stdlib.h>
int main()
{
    sys::graphic_context gc(sys::get_screen_width(), sys::get_screen_height(), "background");
    int texWidth, texHeight, texChannels;
    //   uint8_t *data = stbi_load("init_fs/boot_pic.bmp", &texWidth, &texHeight, &texChannels, STBI_rgb);
    gui::img_bmp bmp = gui::img_bmp("init_fs/background_pic.bmp");
    gc.clear_buffer(sys::pixel(70, 70, 70, 255));
    for (unsigned int x = 0; x < bmp.get_width(); x++)
    {
        for (unsigned int y = 0; y < bmp.get_height(); y++)
        {
            sys::pixel target = ((sys::pixel *)bmp.get_pix_data())[x + y * bmp.get_width()];
            target.a = 255;
            gc.set_pixel(target, x, bmp.get_height() - y);
        }
    }
    gc.set_as_background();
    gc.set_graphic_context_position({0, 0});
    gc.swap_buffer();
    while (true)
    {
        sys::switch_process();
    }
    return 0;
}
