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
    void widget_container::relayout()
    {

        if (list.size() == 0)
        {
            return;
        }
        if (layout_type == layout_type::LAYOUT_VERTICAL)
        {
            size_t layout_reserved_size = 0;
            size_t layout_reserved_count = 0;
            size_t layout_off = 0;
            for (size_t i = 0; i < list.size(); i++)
            {

                if (list[i]->is_manually_sized())
                {
                    layout_reserved_size += list[i]->height();
                    layout_reserved_count++;
                }
            }
            if ((list.size() - layout_reserved_count) != 0)
            {
                layout_off = (height() - layout_reserved_size) / (list.size() - layout_reserved_count);
            }
            size_t current_offset = 0;
            for (size_t i = 0; i < list.size(); i++)
            {
                if (list[i]->is_manually_sized())
                {
                    list[i]->resize(widget_x, widget_y, list[i]->width(), list[i]->height());
                    current_offset += list[i]->height();
                }
                else
                {

                    list[i]->resize(widget_x, widget_y + (current_offset), widget_width, layout_off);
                    current_offset += layout_off;
                }
            }
        }
        if (layout_type == layout_type::LAYOUT_HORIZONTAL)
        {
            size_t layout_off = widget_width / list.size();
            for (size_t i = 0; i < list.size(); i++)
            {
                list[i]->resize(widget_x + (layout_off * i), widget_y, layout_off, widget_height);
            }
        }
    }
    void widget_container::init_widget(void *new_parent)
    {
        parent = new_parent;
    }

    void widget_container::draw_widget(graphic_context &context)
    {
        for (int i = 0; i < list.size(); i++)
        {
            if (list[i]->should_redraw())
            {
                list[i]->draw_widget(context);
            }
        }
    }
} // namespace gui
