
#include <klib/process_message.h>
#include <klib/raw_graphic.h>

namespace sys
{

    uint64_t get_graphic_buffer_addr()
    {
        graphic_buffer_protocol gbp_message = {0};
        gbp_message.data1 = 0;
        gbp_message.data2 = 0;
        gbp_message.data3 = 0;
        gbp_message.request = sys::GET_CURRENT_BUFFER_ADDR;

        uint64_t result = sys::process_message("graphic_buffer_service", (uint64_t)&gbp_message, sizeof(graphic_buffer_protocol)).read();
        return result;
    }

    uint64_t get_screen_width()
    {
        graphic_buffer_protocol gbp_message = {0};
        gbp_message.data1 = 0; // screen width
        gbp_message.data2 = 0;
        gbp_message.data3 = 0;
        gbp_message.request = sys::GET_SCREEN_SIZE;

        uint64_t result = sys::process_message("graphic_buffer_service", (uint64_t)&gbp_message, sizeof(graphic_buffer_protocol)).read();
        return result;
    }

    uint64_t get_screen_height()
    {
        graphic_buffer_protocol gbp_message = {0};
        gbp_message.data1 = 1; // screen height
        gbp_message.data2 = 0;
        gbp_message.data3 = 0;
        gbp_message.request = sys::GET_SCREEN_SIZE;

        uint64_t result = sys::process_message("graphic_buffer_service", (uint64_t)&gbp_message, sizeof(graphic_buffer_protocol)).read();
        return result;
    }
}; // namespace sys
