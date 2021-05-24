

#include <gui/font/basic_8x8_font.h>
#include <gui/graphic_system.h>
#include <gui/raw_graphic.h>
#include <kern/framebuffer.h>
#include <kern/kernel_util.h>
#include <kern/mem_util.h>
#include <kern/mouse_keyboard.h>
#include <kern/process_message.h>
#include <kern/syscall.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

gui::color *front_buffer;
gui::color *back_buffer;
uint64_t real_gbuffer_addr = 0x0;
uint64_t screen_width = 0;
uint64_t screen_height = 0;
sys::file inbuffer;
sys::file outbuffer;
char *buffer;
char *input_buffer;
size_t sbuffer_length = 0;
size_t terminal_width;
size_t terminal_height;
size_t last_key_press;
size_t input_buffer_length;
size_t input_buffer_length_allocated;
void increase(size_t new_size)
{
    if (buffer == nullptr)
    {
        buffer = (char *)malloc(new_size);
        sbuffer_length = new_size;
    }
    else if (sbuffer_length < new_size)
    {
        buffer = (char *)realloc((void *)buffer, new_size);
        sbuffer_length = new_size;
    }
}

size_t line_count()
{

    size_t count = 0;
    for (size_t i = 0; i < sbuffer_length; i++)
    {
        if (buffer[i] == '\n')
        {
            count++;
        }
    }
    return count;
}

size_t render_line_count()
{ // count line wrapped
    size_t count = 0;
    size_t line_char = 0;
    for (size_t i = 0; i < sbuffer_length; i++)
    {
        if (buffer[i] == '\n')
        {
            line_char = 0;
            count++;
        }
        else
        {
            line_char++;
            if (line_char > terminal_width)
            {
                count++;
                line_char = 0;
            }
        }
    }
    return count;
}
void skip_line(size_t &idx)
{

    size_t line_char = 0;
    for (size_t i = idx; i < sbuffer_length; i++)
    {
        if (buffer[i] == '\n')
        {
            line_char = 0;
            idx++;
            return;
        }
        else
        {
            idx++;
            line_char++;
            if (line_char > terminal_width)
            {
                return;
            }
        }
    }
}

void apply_command()
{

    inbuffer.write((uint8_t *)input_buffer, input_buffer_length);
    free(input_buffer);
    input_buffer_length_allocated = 0;
    input_buffer_length = 0;
}

void keyboard_input(char key_char)
{
    if (key_char == 0xe) // back key
    {

        if (input_buffer_length > 0)
        {
            input_buffer_length--;
        }
    }
    else if (key_char != 0)
    {
        input_buffer_length++;
        if (input_buffer_length_allocated == 0)
        {
            input_buffer = (char *)malloc(1);
            input_buffer_length_allocated++;
        }
        else if (input_buffer_length_allocated < input_buffer_length)
        {
            input_buffer = (char *)realloc(input_buffer, input_buffer_length_allocated + 1);
            input_buffer_length_allocated++;
        }
        input_buffer[input_buffer_length - 1] = sys::asciiDefault[key_char];
        if (sys::asciiDefault[key_char] == '\n')
        {
            apply_command();
        }
    }
}
void keyboard_callback()
{

    while (sys::get_current_keyboard_offset() > last_key_press)
    {
        auto press = sys::get_key_press(last_key_press);
        if (press.state)
        {
            keyboard_input(press.button);
        }

        last_key_press++;
    }
}

void update()
{
    keyboard_callback();

    uint64_t nlength = outbuffer.get_file_length();

    if (nlength > sbuffer_length)
    {
        auto last_length = sbuffer_length;
        increase(nlength);
        outbuffer.read((uint8_t *)buffer + last_length, nlength - last_length);
    }
}

void draw_char(int x, int y, uint8_t v, gui::color value)
{
    for (size_t cx = 0; cx < 8; cx++)
    {
        for (size_t cy = 0; cy < 8; cy++)
        {
            if (((font8x8_basic[v][cy] >> cx) & 1) == true)
            {
                back_buffer[(cx + x) + (cy + y) * screen_width] = value;
            }
        }
    }
}

void draw()
{
    gui::color current_col = gui::color(255, 255, 255);
    size_t line_renderer = render_line_count();
    size_t line_offset = 0;
    if ((terminal_height - 3) < line_renderer)
    {
        line_offset = line_renderer - (terminal_height - 3);
    }
    size_t char_index = 0;
    for (int i = 0; i < line_offset; i++)
    {
        skip_line(char_index);
    }
    size_t current_line = 0;
    size_t line_char = 0;
    size_t last_pos = 0;

    for (size_t i = char_index; i < sbuffer_length; i++)
    {

        if (buffer[i] == '\n')
        {
            current_line++;
            line_char = 0;
        }
        else
        {

            line_char++;
            if (line_char > terminal_width)
            {
                line_char = 0;
                current_line++;
                continue;
            }
            else
            {
                draw_char(line_char * 8, current_line * 8, buffer[i] & 0x7f, current_col);
                //   context.draw_basic_char(widget_x + line_char * 8, widget_y + current_line * 8, buffer[i] & 0x7f, current_col);
            }
        }
    }
    for (size_t i = 0; i < input_buffer_length; i++)
    {

        if (input_buffer[i] == '\n')
        {
            current_line++;
            line_char = 0;
        }
        else
        {

            line_char++;
            if (line_char > terminal_width)
            {
                line_char = 0;
                current_line++;
                continue;
            }
            else
            {
                draw_char(line_char * 8, current_line * 8, input_buffer[i] & 0x7f, current_col);
            }
        }
    }
}
int main(int argc, char **argv)
{
    buffer = nullptr;
    input_buffer = nullptr;
    input_buffer_length = 0;
    input_buffer_length_allocated = 0;

    real_gbuffer_addr = sys::get_framebuffer_addr();
    front_buffer = (gui::color *)real_gbuffer_addr;
    screen_width = sys::get_framebuffer_width();
    screen_height = sys::get_framebuffer_height();
    back_buffer = new gui::color[(screen_width + 2) * (screen_height + 2) + 32];
    auto pid = sys::start_programm("initfs/shell.exe");
    if (pid == 0)
    {
        printf("unable to start shell programm ");
        while (true)
        {
        }
    }
    outbuffer = sys::file(sys::get_process_stdf(1, pid));
    inbuffer = sys::file(sys::get_process_stdf(3, pid));
    inbuffer.seek(0);
    outbuffer.seek(0);
    terminal_width = screen_width / 8;
    terminal_height = screen_height / 8;

    while (true)
    {
        for (int i = 0; i < screen_width * screen_height; i++)
        {
            back_buffer[i] = gui::color(0, 0, 0);
        }
        //memset(back_buffer, 0x00, sizeof(gui::color) * screen_width * screen_height);
        update();
        draw();
        swap_buffers(front_buffer, back_buffer, screen_width * screen_height);
    }
}
