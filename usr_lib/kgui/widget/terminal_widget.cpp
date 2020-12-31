#include <kgui/widget.h>
#include <kgui/widget/terminal_widget.h>
#include <klib/kernel_util.h>
#include <klib/mouse_keyboard.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
namespace gui
{

    terminal_widget::terminal_widget(uint64_t x, uint64_t y, uint64_t width, uint64_t heigth, int pid) : col(sys::pixel(0, 0, 0, 0)),
                                                                                                         pbuffer(pid, sys::process_buffer_type::STDOUT)
    {
        pb_lenth = 0;
        sbuffer_length = ((width / 8)) * ((heigth / 8));
        buffer = (char *)malloc(sbuffer_length + 64);
        memset(buffer, 0, sbuffer_length);
        widget_should_draw = true;
        widget_x = x;
        widget_y = y;
        buffer_offset = 0;
        widget_width = width;
        widget_height = heigth;
    }
    void terminal_widget::update_offset()
    {
    }
    void terminal_widget::draw_term_text(sys::graphic_context &context)
    {
        int x = 0;
        int y = 0;
        sys::pixel current_col = sys::pixel(255, 255, 255);
        for (x = 0; x < widget_width / 8; x++)
        {
            for (y = 0; y < widget_height / 8; y++)
            {
                if (buffer[x + y * (widget_width / 8)] != 0)
                {
                    context.draw_basic_char(widget_x + x * 8, widget_y + y * 8, buffer[x + y * (widget_width / 8)] & 0x7f, current_col);
                }
            }
        }
    }
    void terminal_widget::update_widget()
    {
        uint64_t nlength = pbuffer.get_length();

        if (nlength > pb_lenth)
        {
            widget_should_draw = true;

            char *tbuffer = (char *)malloc((nlength - pb_lenth) + 5);

            int rlength = pbuffer.next((uint8_t *)tbuffer, sbuffer_length);
            int offseted_new_size = 0;
            int offseted_x = 0;
            int offseted_y = 0;
            for (int i = 0; i < rlength; i++)
            {
                offseted_x++;
                if (tbuffer[i] == '\n')
                {
                    offseted_x = 0;
                    offseted_y++;
                }
                if (offseted_x > (widget_width / 8))
                {
                    offseted_x = 0;
                    offseted_y++;
                }
            }

            offseted_new_size = offseted_x + offseted_y * (widget_width / 8);
            if (offseted_new_size % (widget_width / 8) != 0)
            {
                offseted_new_size /= (widget_width / 8);
                offseted_new_size += 1;
                offseted_new_size *= (widget_width / 8);
            }
            for (int i = 0; i < sbuffer_length; i++)
            {
                if (offseted_new_size + i > sbuffer_length)
                {
                    buffer[i] = 0;
                }
                else
                {
                    buffer[i] = buffer[i + offseted_new_size];
                }
            }
            int toff = 0;
            int sn_start = sbuffer_length - offseted_new_size;
            if (sn_start < 0)
            {
                sn_start = 0;
            }

            for (int i = sn_start; i < sbuffer_length; i++)
            {
                if (tbuffer[toff] == '\n')
                {
                    toff++;
                    i /= widget_width / 8;
                    i += 1;
                    i *= widget_width / 8;
                    continue;
                }
                buffer[i] = tbuffer[toff];
                toff++;
            }
            pb_lenth += rlength;
            free(tbuffer);
        }
    };
    void terminal_widget::draw_widget(sys::graphic_context &context)
    {
        context.draw_rectangle(widget_x, widget_y, widget_width, widget_height, col);
        draw_term_text(context);
    };
    void terminal_widget::init_widget(void *new_parent){

    };
} // namespace gui
