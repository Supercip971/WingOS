#pragma once
#include <kgui/widget.h>
#include <klib/process_buffer.h>

namespace gui
{

    class terminal_widget : public widget
    {
        sys::pixel col;
        char *buffer;
        uint64_t sbuffer_length = 0;
        uint64_t pb_lenth = 0;
        sys::process_buffer pbuffer;
        int buffer_offset;
        void draw_term_text(sys::graphic_context &context);
        void update_offset();

    public:
        terminal_widget(uint64_t x, uint64_t y, uint64_t width, uint64_t heigth, int pid);

        virtual void update_widget() override;
        virtual void draw_widget(sys::graphic_context &context) override;
        virtual void init_widget(void *new_parent) override;
    };
} // namespace gui
