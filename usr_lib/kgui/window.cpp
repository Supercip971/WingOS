#include <kgui/window.h>
#include <klib/graphic_system.h>
#include <klib/mem_util.h>
namespace gui {
    window::window(const char* name, uint64_t window_width, uint64_t window_height){
        width = window_width;
        height = window_height;
        window_name = name;
        graphic_context = (sys::graphic_context*)sys::service_malloc(sizeof (sys::graphic_context));

        *graphic_context = sys::graphic_context(width, height, window_name);
        graphic_context->clear_buffer({100,100,100,255});
        graphic_context->swap_buffer();
    }

    uint64_t window::start(){
        while(true){


            graphic_context->clear_buffer({100,100,100,255});
            graphic_context->swap_buffer();
        }
    }
}
