#pragma once

#include <kgui/window.h>
namespace gui
{

    class button_widget : public widget
    {
        const char *button_title;
        bool is_hovered;
        uint64_t text_length = 0;
        gui::window *parent;
        bool start_click = false;
        uint64_t clicked = 0;
        void (*click)(uint64_t click_count);

    public:
        button_widget();
        button_widget(uint64_t x, uint64_t y, uint64_t width, uint64_t heigth, const char *title) __attribute__((__target__("no-sse")));
        constexpr void set_click_callback(void (*callback)(uint64_t click_count))
        {
            click = callback;
        }
        virtual void update_widget() override;
        virtual void draw_widget(sys::graphic_context &context) override;
        virtual void init_widget(void *new_parent) override;
    };
} // namespace gui
