#include <gui/widget.h>
#include <gui/widget/rectangle.h>

namespace gui
{
    rectangle_widget::rectangle_widget() : col(pixel(0, 0, 0, 0))
    {
        widget_should_draw = true;
    }
    rectangle_widget::rectangle_widget(uint64_t x, uint64_t y, uint64_t width, uint64_t heigth, pixel color) : col(color)
    {

        widget_should_draw = true;
        widget_x = x;
        widget_y = y;
        widget_width = width;
        widget_height = heigth;
    }
    void rectangle_widget::update_widget(){

    };
    void rectangle_widget::draw_widget(graphic_context &context)
    {
        context.draw_rectangle(widget_x, widget_y, widget_width, widget_height, col);
    };
    void rectangle_widget::init_widget(void *new_parent){

    };
} // namespace gui
