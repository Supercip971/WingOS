#include <kgui/widget.h>
#include <klib/mem_util.h>
namespace gui
{

    widget::widget()
    {
    }

    bool widget::is_position_inside_widget(const sys::pos pos)
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
        list = (gui::widget **)sys::service_malloc(length * sizeof(uint64_t));
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

    void widget_list::update_all()
    {

        for (size_t i = 0; i < list_length; i++)
        {
            if (list[i] != nullptr)
            {
                list[i]->update_widget();
            }
        }
    }
    void widget_list::draw_all(sys::graphic_context &context)
    {

        for (size_t i = 0; i < list_length; i++)
        {
            if (list[i] != nullptr)
            {
                list[i]->draw_widget(context);
            }
        }
    }
} // namespace gui
