#include <gui/widget.h>
#include <gui/widget/button.h>
#include <kern/mouse_keyboard.h>
#include <stdlib.h>
#include <string.h>
namespace gui
{
    button_widget::button_widget()
    {
        button_title = "null";
    }
    button_widget::button_widget(uint64_t x, uint64_t y, uint64_t width, uint64_t heigth, const char *title)
    {
        widget_x = x;
        widget_y = y;
        widget_width = width;
        widget_height = heigth;
        button_title = title;
        text_length = strlen(title) * 8;
        is_hovered = false;
        click = nullptr;
    }
    void button_widget::update_widget()
    {
        bool has_been_hovered = false;
        if (parent != nullptr)
        {
            uint32_t x = parent->get_mouse_pos_relative_x();
            if (x > widget_x)
            {
                if (x < widget_x + widget_width)
                {

                    uint32_t y = parent->get_mouse_pos_relative_y();
                    if (y > widget_y)
                    {
                        if (y <= widget_y + widget_height)
                        {
                            if (parent->is_window_front())
                            {
                                has_been_hovered = true;
                            }
                        }
                    }
                }
            }
        }
        if (!is_hovered && has_been_hovered)
        {

            widget_should_draw = true;
        }
        if (is_hovered && !has_been_hovered)
        {

            widget_should_draw = true;
        }
        is_hovered = has_been_hovered;
        if (click != nullptr)
        {

            if (is_hovered && sys::get_mouse_button(sys::GET_MOUSE_LEFT_CLICK))
            {
                if (start_click == false)
                {
                    widget_should_draw = true;
                    start_click = true;
                    click(clicked++);
                }
            }
            else
            {
                start_click = false;
            }
        }
    };
    void button_widget::draw_widget(graphic_context &context)
    {
        if (is_hovered)
        {
            context.draw_rounded_rectangle(4, widget_x, widget_y, widget_width, widget_height, {115, 70, 70, 255});
        }
        else
        {
            context.draw_rounded_rectangle(4, widget_x, widget_y, widget_width, widget_height, {80, 80, 80, 255});
        }
        //  context.draw_rounded_rectangle(6, widget_x, widget_y, widget_width, widget_height, {200, 200, 200, 255});
        context.draw_basic_string(widget_x + (widget_width / 2) - text_length / 2, widget_y + (widget_height / 2) - 4, button_title, color(255, 255, 255));
    };
    void button_widget::init_widget(void *new_parent)
    {
        parent = reinterpret_cast<gui::window *>(new_parent);
    };
} // namespace gui
