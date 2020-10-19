#include <kgui/widget/movable_context_widget.h>
#include <klib/mouse.h>
#include <stdio.h>
namespace gui
{

    movable_context_widget::movable_context_widget()
    {
    }
    movable_context_widget::movable_context_widget(uint64_t x, uint64_t y, uint64_t width, uint64_t height, gui::window *target)
    {
        widget_x = x;
        widget_y = y;
        widget_width = width;
        widget_height = height;
        parent = target;
        dragging = false;
    }

    void movable_context_widget::update_widget()
    {

        if (dragging == false)
        {
            if (sys::get_mouse_button(sys::GET_MOUSE_LEFT_CLICK))
            {
                sys::pos position;
                position.x = parent->get_mouse_pos_relative_x();
                position.y = parent->get_mouse_pos_relative_y();
                if (is_position_inside_widget(position))
                {

                    sys::pos rposition;
                    rposition.x = sys::get_mouse_x();
                    rposition.y = sys::get_mouse_y();
                    start_dragging_pos = rposition;
                    current_dragging_pos = rposition;
                    dragging = true;
                    drag_time = 0;
                }
            }
        }
        else
        {
            if (!sys::get_mouse_button(sys::GET_MOUSE_LEFT_CLICK))
            {
                dragging = false;
                return;
            }
            drag_time++;
            sys::pos position;
            position.x = sys::get_mouse_x();
            position.y = sys::get_mouse_y();

            if (position.x == current_dragging_pos.x && position.y == current_dragging_pos.y)
            {
                return;
            }
            int32_t result_x = (int)position.x - (int)current_dragging_pos.x;
            int32_t result_y = (int)position.y - (int)current_dragging_pos.y;
            current_dragging_pos = position;

            sys::pos p = parent->get_window_position();
            p.x += result_x;
            p.y += result_y;
            if (p.x < 0)
            {
                p.x = 0;
            }
            if (p.y < 0)
            {
                p.y = 0;
            }
            parent->set_window_position(p);
        }
    }

    void movable_context_widget::draw_widget(sys::graphic_context &context)
    {
    }
    void movable_context_widget::init_widget(void *new_parent)
    {
    }

} // namespace gui
