#include <kgui/widget/button_widget.h>
#include <kgui/widget.h>
#include <string.h>
#include <stdlib.h>
namespace gui {
    button_widget::button_widget() {
        button_title = "null";
    }
    button_widget::button_widget(uint64_t x, uint64_t y, uint64_t width, uint64_t heigth, const char* title){
        widget_x = x;
        widget_y = y;
        widget_width = width;
        widget_height = heigth;
        button_title = title;
        text_length = strlen(title) * 8;
    }
    void button_widget::update_widget() {
    };
    void button_widget::draw_widget(sys::graphic_context& context) {
        context.draw_rectangle(widget_x, widget_y, widget_width, widget_height, {80,80,80,255});
        context.draw_basic_string(widget_x + (widget_width / 2) - text_length / 2, widget_y + (widget_height/2) - 4, button_title, sys::pixel(255,255,255));
    };
    void button_widget::init_widget(void* new_parent){
    };
}
