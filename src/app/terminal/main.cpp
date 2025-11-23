#include <string.h>
#include "libcore/str_writer.hpp"

#include "arch/generic/syscalls.h"
#include "iol/wingos/space.hpp"
#include "iol/wingos/syscalls.h"
#include "libcore/alive.hpp"
#include "libcore/fmt/log.hpp"
#include "mcx/mcx.hpp"
#include "protocols/compositor/compositor.hpp"
#include "protocols/compositor/window.hpp"
#include "protocols/init/init.hpp"
#include "protocols/vfs/vfs.hpp"
#include "wingos-headers/syscalls.h"

#define ASCII_FONT_IMPLEMENTATION
#include "ascii_font.h"

int main(int, char **)
{
    log::log$("hello world from vfs app!");

    auto wdw = prot::WindowConnection::create(true).unwrap();

    auto asset = wdw.get_framebuffer().unwrap();

    void *bb = asset.ptr();

    auto size = wdw.get_attribute_size().unwrap();

    void *fb = malloc(size.width * size.height * 4);

    log::log$("window size: {}x{}", size.width, size.height);


    core::WStr wstr;

    size_t frame = 0;
    while (true)
    {

        uint32_t r = frame % 256;
        uint32_t g = (frame / 256) % 256;
        uint32_t b = (frame / (256 * 256)) % 256;

        for (size_t i = 0; i < size.width * size.height; i++)
        {

            size_t x = (i % size.width) + frame;
            size_t y = (i / size.width) + frame;
            b = x ^ y;
            r = (y * 2) ^ (x * 2);
            g = (y * 4) ^ (x * 4);

            ((uint32_t *)fb)[i] = 0xff000000 | (r << 16) | (g << 8) | (b);
        }

        // draw some text
        
        wstr.release();
        size_t text_len = core::Str(text).len();

        for (size_t i = 0; i < text_len; i++)
        {
            char c = text[i];
            for (size_t cy = 0; cy < ASCII_FONT_HEIGHT; cy++)
            {
                for (size_t cx = 0; cx < ASCII_FONT_WIDTH; cx++)
                {
                    size_t px = 50 + i * ASCII_FONT_WIDTH + cx;
                    size_t py = 50 + cy;
                    if (px >= size.width || py >= size.height)
                        continue;

                    uint32_t pixel_on = (ascii_font[(int)c][cy][cx]);
                    ((uint32_t *)fb)[py * size.width + px] =
                        0xff000000 | (pixel_on << 16) | (pixel_on << 8) | (pixel_on);
                }
            }
        }

        memcpy(bb, fb, size.width * size.height * 4);
        wdw.swap_buffers();
        frame++;

        //    log::log$("swapped buffers: {}", frame);
    }
}