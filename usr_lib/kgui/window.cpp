#include <kgui/window.h>
#include <klib/graphic_system.h>
#include <klib/mem_util.h>
namespace gui {
    window::window(const char* name, uint64_t window_width, uint64_t window_height) : graphic_context(window_width, window_height, name){
        width = window_width;
        height = window_height;
        window_name = name;

        graphic_context.clear_buffer(sys::pixel(100,100,100,0));
        graphic_context.swap_buffer();
        lst = widget_list();
        lst.init(128); // wax 128 widget may be increase later, but you will be able to put widget list as a widget so meh
    }

    uint64_t window::start(){
        while(true){
            lst.update_all();
            graphic_context.clear_buffer({100,100,100,255});
            graphic_context.draw_rectangle(0,0,width,20, sys::pixel(80,80,80));
            graphic_context.draw_basic_string((width / 2) - (sys::get_basic_font_width_text(window_name) / 2),20 / 2 - (8 / 2),window_name, sys::pixel(255,255,255));
            lst.draw_all(graphic_context);
            graphic_context.swap_buffer();
        }
    }
    void window::add_widget(widget* wdget){
       lst.add_widget(wdget);
       wdget->init_widget();
    }
}
