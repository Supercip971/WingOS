#include <gui/graphic_system.h>
#include <gui/widget/movable.h>
#include <gui/window.h>
#include <kern/kernel_util.h>
#include <kern/mem_util.h>
#include <kern/mouse_keyboard.h>
#include <kern/syscall.h>
#include <stdio.h>
namespace gui
{
    window::window(const char *name, uint64_t window_width, uint64_t window_height) : window_graphic_context(window_width, window_height, name)
    {
        width = window_width;
        height = window_height;
        window_name = name;

        window_graphic_context.clear_buffer(color(100, 100, 100, 0));
        window_graphic_context.swap_buffer();
        lst = new widget_container();
        lst->init_widget(this); // wax 128 widget may be increase later, but you will be able to put widget list as a widget so meh
        lst->resize(0, 0, width, height);
        header_widget_movable = new gui::movable_context_widget(0, 0, width, 20, this);
        has_at_least_one_redraw = true;
        add_widget(header_widget_movable);
    };

    void window::resize(uint64_t new_width, uint64_t new_height)
    {
        width = new_width;
        height = new_height;
        window_graphic_context.resize(new_width, new_height);
        lst->resize(0, 0, width, height);
    }
    void window::update_input()
    {
        while (true)
        {
            auto v = window_graphic_context.update_input_info();
            if (v.info_type == 0)
            {
                break;
            }
            else
            {
                lst->callback_update(v);
            }
        }
    }

    uint64_t window::start()
    {

        window_graphic_context.clear_buffer({100, 100, 100, 0});
        window_graphic_context.draw_rounded_rectangle(4, 0, 0, width, height, back_window_color);
        window_graphic_context.draw_rounded_rectangle(4, 0, 0, width, 20, window_front_color);
        window_graphic_context.draw_basic_string((width / 2) - (get_basic_font_width_text(window_name) / 2), 20 / 2 - (8 / 2), window_name, color(255, 255, 255));
        lst->draw_widget(window_graphic_context);
        window_graphic_context.swap_buffer();
        bool start_click = false;
        while (true)
        {

            if (sys::get_mouse_button(sys::GET_MOUSE_LEFT_CLICK) && start_click)
            {

                start_click = false;
            }
            else if (!sys::get_mouse_button(sys::GET_MOUSE_LEFT_CLICK))
            {
                start_click = true;
            }
            lst->update_widget();
            has_at_least_one_redraw = lst->should_redraw();

            if (has_at_least_one_redraw)
            {
                window_graphic_context.clear_buffer({100, 100, 100, 0});
                window_graphic_context.draw_rounded_rectangle(4, 0, 0, width, height, window_front_color);
                window_graphic_context.draw_rounded_rectangle(4, 1, 1, width - 2, height - 2, back_window_color);
                window_graphic_context.draw_rounded_rectangle(4, 0, 0, width, 20, window_front_color);
                window_graphic_context.draw_basic_string((width / 2) - (get_basic_font_width_text(window_name) / 2), 20 / 2 - (8 / 2), window_name, color(255, 255, 255));
                lst->draw_widget(window_graphic_context);
                update_input(); // we need to do that JUST before the swap buffer
                window_graphic_context.swap_buffer();
            }
            else
            {
                update_input();
            }

        }
    }
    void window::add_widget(widget *wdget)
    {
        lst->add_widget(wdget);
        wdget->init_widget(this);
    }

    int32_t window::get_mouse_pos_relative_x()
    {
        int32_t wx = window_graphic_context.get_graphic_context_position().rpos.x;

        return sys::get_mouse_x() - wx;
    }
    int32_t window::get_mouse_pos_relative_y()
    {
        int32_t wy = window_graphic_context.get_graphic_context_position().rpos.y;

        return sys::get_mouse_y() - wy;
    }

    pos window::get_window_position()
    {
        sys::raw_pos p = window_graphic_context.get_graphic_context_position();
        return pos(p.rpos.x, p.rpos.y);
    }
    void window::set_window_position(uint32_t x, uint32_t y)
    {
        window_graphic_context.set_graphic_context_position({x, y});
    }
    void window::set_window_position(pos position)
    {
        uint32_t x = position.x;
        uint32_t y = position.y;
        set_window_position(x, y);
    }

    bool window::is_mouse_on_window()
    {

        return (window_graphic_context.get_window_id() == sys::sys$get_process_global_data(0, "initfs/graphic_service.exe"));
    }
    bool window::is_window_front()
    {

        return (window_graphic_context.is_on_top());
    }
} // namespace gui
