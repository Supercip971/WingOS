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
    window::window(const char *name, uint64_t window_width, uint64_t window_height) : graphic_context(window_width, window_height, name)
    {
        width = window_width;
        height = window_height;
        window_name = name;

        graphic_context.clear_buffer(sys::pixel(100, 100, 100, 0));
        graphic_context.swap_buffer();
        lst = widget_list();
        lst.init(128); // wax 128 widget may be increase later, but you will be able to put widget list as a widget so meh
        header_widget_movable = new gui::movable_context_widget(0, 0, width, 20, this);
        has_at_least_one_redraw = true;
        add_widget(header_widget_movable);
    };
    uint64_t window::start()
    {
        graphic_context.clear_buffer({100, 100, 100, 0});
        graphic_context.draw_rounded_rectangle(4, 0, 0, width, height, sys::pixel(100, 100, 100));
        graphic_context.draw_rounded_rectangle(4, 0, 0, width, 20, sys::pixel(75, 75, 75));
        graphic_context.draw_basic_string((width / 2) - (sys::get_basic_font_width_text(window_name) / 2), 20 / 2 - (8 / 2), window_name, sys::pixel(255, 255, 255));
        lst.draw_all(graphic_context);
        graphic_context.swap_buffer();
        bool start_click = false;
        while (true)
        {
            if (sys::get_mouse_button(sys::GET_MOUSE_LEFT_CLICK) && start_click)
            {

                if (graphic_context.is_mouse_inside())
                {
                    if (is_mouse_on_window())
                    {
                        graphic_context.set_on_top();
                    }
                }
                start_click = false;
            }
            else if (!sys::get_mouse_button(sys::GET_MOUSE_LEFT_CLICK))
            {
                start_click = true;
            }
            has_at_least_one_redraw = lst.update_all();

            if (has_at_least_one_redraw)
            {
                graphic_context.clear_buffer({100, 100, 100, 0});
                graphic_context.draw_rounded_rectangle(4, 0, 0, width, height, sys::pixel(100, 100, 100));
                graphic_context.draw_rounded_rectangle(4, 0, 0, width, 20, sys::pixel(75, 75, 75));
                graphic_context.draw_basic_string((width / 2) - (sys::get_basic_font_width_text(window_name) / 2), 20 / 2 - (8 / 2), window_name, sys::pixel(255, 255, 255));
                lst.draw_all(graphic_context);
                graphic_context.swap_buffer();
            }

            sys::switch_process();
        }
    }
    void window::add_widget(widget *wdget)
    {
        lst.add_widget(wdget);
        wdget->init_widget(this);
    }

    int32_t window::get_mouse_pos_relative_x()
    {
        int32_t wx = graphic_context.get_graphic_context_position().rpos.x;

        return sys::get_mouse_x() - wx;
    }
    int32_t window::get_mouse_pos_relative_y()
    {
        int32_t wy = graphic_context.get_graphic_context_position().rpos.y;

        return sys::get_mouse_y() - wy;
    }

    sys::pos window::get_window_position()
    {
        sys::raw_pos p = graphic_context.get_graphic_context_position();
        return sys::pos(p.rpos.x, p.rpos.y);
    }
    void window::set_window_position(uint32_t x, uint32_t y)
    {
        graphic_context.set_graphic_context_position({x, y});
    }
    void window::set_window_position(sys::pos position)
    {
        uint32_t x = position.x;
        uint32_t y = position.y;
        set_window_position(x, y);
    }

    bool window::is_mouse_on_window()
    {

        return (graphic_context.get_window_id() == sys::sys$get_process_global_data(0, "init_fs/graphic_service.exe"));
    }
    bool window::is_window_front()
    {

        return (graphic_context.is_on_top());
    }
} // namespace gui
