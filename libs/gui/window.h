#pragma once
#include <gui/graphic_system.h>
#include <gui/widget.h>
#include <stdint.h>
namespace gui
{

    constexpr color window_front_color = color(24, 24, 24, 255);
    constexpr color highlight_wingow_color = color(59, 130, 246, 255);
    constexpr color text_window_color = color(249, 250, 251, 255);
    constexpr color back_window_color = color(12, 12, 12, 255);

    class window
    {
        const char *window_name;
        graphic_context window_graphic_context;
        uint64_t width;
        uint64_t height;
        widget_container *lst;
        bool has_at_least_one_redraw = true;
        uint64_t current_tick;
        widget *header_widget_movable;
        void update_input();

    public:
        // create a simple window
        // if you don't want a update_function give just a nullptr,
        // it will just skip the update
        // todo: add a after_draw_update() and a before_draw_update()
        window(const char *name, uint64_t window_width, uint64_t window_height) __attribute__((__target__("no-sse")));
        void add_widget(widget *wdget);
        bool is_mouse_on_window();
        bool is_window_front();
        // get mouse position inside the window
        int32_t get_mouse_pos_relative_x();
        int32_t get_mouse_pos_relative_y();
        pos get_window_position();
        void set_window_position(uint32_t x, uint32_t y);
        void set_window_position(pos position);
        uint64_t start();
    };
} // namespace gui
