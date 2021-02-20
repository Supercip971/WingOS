#include <gui/widget.h>
#include <kern/mem_util.h>
#include <stdlib.h>
namespace gui
{

    widget::widget()
    {
    }

    bool widget::is_position_inside_widget(const pos pos)
    {
        if (pos.x >= widget_x)
        {
            if (pos.y >= widget_y)
            {
                if (pos.y <= widget_height + widget_y)
                {
                    if (pos.x <= widget_width + widget_x)
                    {
                        return true;
                    }
                }
            }
        }
        return false;
    }
    void widget_list::init(size_t length)
    {
        list = (gui::widget **)malloc(length * sizeof(uint64_t));
        for (size_t i = 0; i < length; i++)
        {
            list[i] = nullptr;
        }
        list_length = length;
    }

    void widget_list::add_widget(widget *widget)
    {
        for (size_t i = 0; i < list_length; i++)
        {
            if (list[i] == nullptr)
            {
                list[i] = widget;
                return;
            }
        }
    }

    bool widget_list::update_all()
    {
        bool should_redraw = false;
        for (size_t i = 0; i < list_length; i++)
        {
            if (list[i] != nullptr)
            {

                list[i]->update_widget();
                if (should_redraw == false && list[i]->should_redraw())
                {
                    should_redraw = true;
                }
            }
        }
        return should_redraw;
    }
    void widget_list::draw_all(graphic_context &context)
    {

        for (size_t i = 0; i < list_length; i++)
        {
            if (list[i] != nullptr)
            {
                if (list[i]->should_redraw())
                {
                    list[i]->set_should_redraw(false);

                    list[i]->draw_widget(context);
                }
            }
        }
    }
} // namespace gui
