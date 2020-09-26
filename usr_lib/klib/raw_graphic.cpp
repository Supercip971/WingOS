#pragma once
#include <klib/raw_graphic.h>
#include <klib/process_message.h>

namespace sys{

    uint64_t get_graphic_buffer_addr(){
        graphic_buffer_protocol gbp_message = {0};
        gbp_message.data1 = 0;
        gbp_message.data2 = 0;
        gbp_message.data3 = 0;
        gbp_message.data3 = sys::GET_CURRENT_BUFFER_ADDR;

        uint64_t result = sys::process_message("graphic_buffer_service", (uint64_t)&gbp_message, sizeof (graphic_buffer_protocol)).read();
        return result;
    }
};
