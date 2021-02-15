#include <gui/graphic_system.h>
#include <gui/window.h>
#include <kern/kernel_util.h>
#include <kern/mem_util.h>
#include <kern/process_message.h>
#include <kern/syscall.h>
//#include <feather_language_lib/feather.h>
#include <gui/img_bmp.h>
#include <gui/raw_graphic.h>
#include <gui/widget/button.h>
#include <gui/widget/rectangle.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int main(int argc, char **argv)
{
    gui::graphic_context gc(sys::get_screen_width(), sys::get_screen_height(), "background");
    int texWidth, texHeight, texChannels;
    //   uint8_t *data = stbi_load("initfs/boot_pic.bmp", &texWidth, &texHeight, &texChannels, STBI_rgb);
    gui::img_bmp bmp = gui::img_bmp("initfs/background_pic.bmp");
    gc.clear_buffer(gui::pixel(70, 70, 70, 255));
    for (unsigned int x = 0; x < bmp.get_width(); x++)
    {
        for (unsigned int y = 0; y < bmp.get_height(); y++)
        {
            gui::pixel target = ((gui::pixel *)bmp.get_pix_data())[x + y * bmp.get_width()];
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
