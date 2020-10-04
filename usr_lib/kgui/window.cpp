#include <kgui/window.h>
#include <klib/graphic_system.h>
#include <klib/mem_util.h>
namespace gui {
    window::window(const char* name, uint64_t window_width, uint64_t window_height) : graphic_context(window_width, window_height, name){
        width = window_width;
        height = window_height;
        window_name = name;
//        *graphic_context = sys::graphic_context(width, height, window_name); // <=== c'est ici


        graphic_context.clear_buffer(sys::pixel(100,100,100,0));
        graphic_context.swap_buffer();
    }

    uint64_t window::start(){
        while(true){


            graphic_context.clear_buffer({100,100,100,255});
            graphic_context.swap_buffer();
        }
    }
}
