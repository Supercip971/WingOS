#include <gui/widget.h>
#include <gui/widget/terminal_widget.h>
#include <kern/kernel_util.h>
#include <kern/mouse_keyboard.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
namespace gui
{

    terminal_widget::terminal_widget(size_t x, size_t y, size_t width, size_t heigth, int pid) : col(sys::pixel(0, 0, 0, 0)),
                                                                                                 outbuffer(sys::get_process_stdf(1, pid)),
                                                                                                 inbuffer(sys::get_process_stdf(3, pid))
    {
        pb_lenth = 0;
        terminal_width = width / 8;
        terminal_height = heigth / 8;
        sbuffer_length = 0;
        buffer = nullptr;
        widget_should_draw = true;
        widget_x = x;
        widget_y = y;
        buffer_offset = 0;
        widget_width = width;
        widget_height = heigth;
        last_key_press = sys::get_current_keyboard_offset();
    }
    void terminal_widget::increase(size_t new_size)
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
    size_t terminal_widget::line_count()
    { // count of '\n'
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
    size_t terminal_widget::render_line_count()
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
    void terminal_widget::update_offset()
    {
    }
    void terminal_widget::skip_line(size_t &idx)
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
    void terminal_widget::draw_term_text(sys::graphic_context &context)
    {
        int x = 0;
        int y = 0;
        sys::pixel current_col = sys::pixel(255, 255, 255);
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
                    context.draw_basic_char(widget_x + line_char * 8, widget_y + current_line * 8, buffer[i] & 0x7f, current_col);
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
                    context.draw_basic_char(widget_x + line_char * 8, widget_y + current_line * 8, input_buffer[i] & 0x7f, current_col);
                }
            }
        }
    }
    void terminal_widget::update_widget()
    {
        uint64_t nlength = outbuffer.get_file_length();

        if (nlength > sbuffer_length)
        {
            auto last_length = sbuffer_length;
            increase(nlength);
            outbuffer.read((uint8_t *)buffer + last_length, nlength - last_length);
            widget_should_draw = true;
        }
        while (sys::get_current_keyboard_offset() > last_key_press)
        {
            auto press = sys::get_key_press(last_key_press);
            if (press.state && press.button == 0xe) // back key
            {

                if (input_buffer_length > 0)
                {
                    input_buffer_length--;
                    widget_should_draw = true;
                }
            }
            else if (press.state && press.button != 0)
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
                input_buffer[input_buffer_length - 1] = sys::asciiDefault[press.button];
                if (sys::asciiDefault[press.button] == '\n')
                {
                    apply_command();
                }
                widget_should_draw = true;
            }
            last_key_press++;
        }
    };
    void terminal_widget::apply_command()
    {
        inbuffer.write((uint8_t *)input_buffer, input_buffer_length);
        free(input_buffer);
        input_buffer_length_allocated = 0;
        input_buffer_length = 0;
    }
    void terminal_widget::draw_widget(sys::graphic_context &context)
    {
        context.draw_rectangle(widget_x, widget_y, widget_width, widget_height, col);
        draw_term_text(context);
    };
    void terminal_widget::init_widget(void *new_parent){

    };
} // namespace gui
