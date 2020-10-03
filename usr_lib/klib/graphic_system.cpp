#include <klib/graphic_system.h>
#include <klib/process_message.h>
#include <klib/kernel_util.h>
#include <klib/mem_util.h>
#include <string.h>

namespace sys {
    graphic_context::graphic_context(uint64_t width, uint64_t height, const char* name){
        context_height = height;
        context_width = width;
        uint64_t name_length = strlen(name) +1;
        context_name = (char*)sys::service_malloc(name_length);
        for(int i =0; i <= name_length; i++){
            context_name[i] = name[i];
        }

        sys::graphic_system_service_protocol tolaunch = {0};
        tolaunch.request_type = sys::GRAPHIC_SYSTEM_REQUEST::CREATE_WINDOW;
        tolaunch.create_window_info.height = height;
        tolaunch.create_window_info.width = width;
        wid = sys::process_message("init_fs/graphic_service.exe", (uint64_t)&tolaunch, sizeof (sys::graphic_system_service_protocol)).read();
        sys::graphic_system_service_protocol get_bbuffer = {0};
        get_bbuffer.request_type = sys::GRAPHIC_SYSTEM_REQUEST::GET_WINDOW_BACK_BUFFER;
        get_bbuffer.get_request.window_handler_code = wid;
        back_buffer = (sys::pixel*)sys::process_message("init_fs/graphic_service.exe", (uint64_t)&get_bbuffer , sizeof (sys::graphic_system_service_protocol)).read();
        uint64_t t = 0;
      

    }

    void graphic_context::clear_buffer(const pixel color){
        const uint64_t context_length = (context_width * context_height);
        raw_clear_buffer(back_buffer, context_length, color);
    }
    void graphic_context::swap_buffer(){
        sys::graphic_system_service_protocol swap_request = {0};
        swap_request.request_type = sys::GRAPHIC_SYSTEM_REQUEST::SWAP_WINDOW_BUFFER;
        swap_request.get_request.window_handler_code = wid;
        uint64_t result = sys::process_message("init_fs/graphic_service.exe", (uint64_t)&swap_request , sizeof (sys::graphic_system_service_protocol)).read();

    }
    void swap_buffer(sys::pixel* buffer1, const sys::pixel* buffer2, uint64_t buffer_length){
        uint64_t buffer_length_r64 = buffer_length / 2;
        uint64_t* to = (uint64_t*)buffer1;
        const uint64_t* from = (const uint64_t*)buffer2;
        for(uint64_t i = 0; i < buffer_length_r64; i++){
            to[i] = from[i];
        }
    }

    void raw_clear_buffer(sys::pixel* buffer, uint64_t size, sys::pixel value){
        const uint64_t msize = size / 2; // copy uint64_t
        const uint64_t rsize = size % 2;
        uint64_t* conv_buffer = (uint64_t*)buffer;
        const uint64_t v = (uint64_t)value.pix | ((uint64_t)value.pix << 32);
        for(uint64_t i = 0;i < msize; i++){
            conv_buffer[i] = v;
        }for(uint64_t i = msize*2; i < msize*2 + rsize; i++){
            buffer[i].pix = value.pix;
        }
    }
}
