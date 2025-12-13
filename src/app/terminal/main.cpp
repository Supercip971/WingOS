#include <stddef.h>
#include <string.h>

#include "libcore/fmt/fmt_str.hpp"
#include "libcore/str_writer.hpp"

#include "arch/generic/syscalls.h"
#include "iol/wingos/execute.hpp"
#include "iol/wingos/space.hpp"
#include "iol/wingos/syscalls.h"
#include "libcore/alive.hpp"
#include "libcore/fmt/log.hpp"
#include "mcx/mcx.hpp"
#include "protocols/compositor/compositor.hpp"
#include "protocols/compositor/window.hpp"
#include "protocols/init/init.hpp"
#include "protocols/pipe/pipe.hpp"
#include "protocols/vfs/vfs.hpp"
#include "wingos-headers/startup.hpp"
#include "wingos-headers/syscalls.h"

#define ASCII_FONT_IMPLEMENTATION
#include "ascii_font.h"

size_t cursor_y = 0;
size_t cursor_x = 0;
bool skip_command = false;
char display[78][22] = {};

void add_str(core::Str const &str)
{

    for (size_t i = 0; i < str.len(); i++)
    {

        if (str[i] == 0)
        {
            continue;
        }
        if (str[i] == 'm' && skip_command)
        {
            skip_command = false;
            continue;
        }
        if (str[i] == '\033')
        {
            skip_command = true;
            continue;
        }
        if (skip_command)
        {
            continue;
        }

        if (str[i] == '\n')
        {
            cursor_y++;
            cursor_x = 0;
        }
        else

        {
            display[cursor_x][cursor_y] = str[i];
            cursor_x++;
            if (cursor_x >= 78)
            {
                cursor_x = 0;
                cursor_y++;
            }
        }
        if (cursor_y >= 22)
        {
            // scroll up
            for (size_t y = 1; y < 22; y++)
            {
                for (size_t x = 0; x < 78; x++)
                {
                    display[x][y - 1] = display[x][y];
                }
            }
            // clear last line
            for (size_t x = 0; x < 78; x++)
            {
                display[x][21] = ' ';
            }
            cursor_y = 21;
        }
    }
}

void draw_char(void *fb, size_t x, size_t y, char c, size_t window_width)
{
    for (size_t cy = 0; cy < ASCII_FONT_HEIGHT; cy++)
    {
        for (size_t cx = 0; cx < ASCII_FONT_WIDTH; cx++)
        {
            size_t px = x + cx;
            size_t py = y + cy;

            uint32_t pixel_on = (ascii_font[(int)c][cy][cx]);
            ((uint32_t *)fb)[py * window_width + px] =
                0xff000000 | (pixel_on << 16) | (pixel_on << 8) | (pixel_on);
        }
    }
}
int main(int, char **)
{

    cursor_x = 0;
    cursor_y = 0;
    for (size_t i = 0; i < 22; i++)
    {
        for (size_t j = 0; j < 78; j++)
        {
            display[j][i] = ' ';
        }
    }
    log::log$("hello world from vfs app!");

    auto wdw = prot::WindowConnection::create(true).unwrap();

    auto asset = wdw.get_framebuffer().unwrap();

    void *bb = asset.ptr();

    auto size = wdw.get_attribute_size().unwrap();

    void *fb = malloc(size.width * size.height * 4);

    log::log$("window size: {}x{}", size.width, size.height);

    core::WStr wstr = {};

    // size_t frame = 0;

    Wingos::Space space = Wingos::Space::self().create_space();

    prot::Duplex pipes = (prot::Duplex::create(
                              space,
                              Wingos::Space::self(),
                              0)
                              .unwrap());

    auto receiver_pipe = (prot::ReceiverPipe::from(core::move(pipes.connection_receiver))
                              .unwrap());

    StartupInfo args = {};
    args.stdout_handle = pipes.connection_sender.handle;
    execute_program_from_path(space, "/bin/hello-wingos", args);

    wstr = core::WStr::own((char *)malloc(256), 0, 256);

    while (true)
    {
        memset(fb, 0, size.width * size.height * 4);

        // draw some text

        for (size_t i = 0; i < 22; i++)
        {
            for (size_t j = 0; j < 78; j++)
            {
                draw_char(fb, (j + 1) * (ASCII_FONT_WIDTH), (i + 1) * (ASCII_FONT_HEIGHT), display[j][i], size.width);
            }
        }

        memcpy(bb, fb, size.width * size.height * 4);
        wdw.swap_buffers();

        //        fmt::format_to_str(wstr, "uwu Terminal App - Frame {} (Hello world <3 !)\n", frame);

        // try receive from pipe

        while (true)
        {

            auto res = receiver_pipe.receive_message();
            if (!res.is_error())
            {
                auto msg = res.take();
                add_str(core::Str((const char *)msg.raw_buffer, msg.len));
            }
            else
            {
                break;
            }
        }

        // wstr.clear();
        //     log::log$("swapped buffers: {}", frame);
    }
}