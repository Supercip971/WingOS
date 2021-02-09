#pragma once
#include <gui/widget.h>
#include <kern/file.h>
#include <kern/process_buffer.h>
#include <utils/wvector.h>
namespace gui
{

    class terminal_widget : public widget
    {
        size_t line_count();
        size_t render_line_count();
        sys::pixel col;
        char *buffer;
        char *input_buffer;
        size_t input_buffer_length;
        size_t input_buffer_length_allocated;
        void add_input_key(char key);

        size_t sbuffer_length = 0;
        size_t pb_lenth = 0;
        size_t terminal_width = 0;
        size_t terminal_height = 0;
        sys::file outbuffer;
        sys::file inbuffer;
        size_t last_key_press = 0;
        int buffer_offset;
        void draw_term_text(sys::graphic_context &context);
        void update_offset();
        void increase(size_t new_size);
        void skip_line(size_t &idx);
        void apply_command();
        char get_buffer(size_t idx)
        {
            if (idx > sbuffer_length)
            {
                return input_buffer[idx - sbuffer_length];
            }
            else
            {
                return buffer[idx];
            }
        }
        size_t get_buffer_size()
        {
            return sbuffer_length + input_buffer_length;
        };

    public:
        terminal_widget(size_t x, size_t y, size_t width, size_t heigth, int pid);

        virtual void update_widget() override;
        virtual void draw_widget(sys::graphic_context &context) override;
        virtual void init_widget(void *new_parent) override;
    };
} // namespace gui
