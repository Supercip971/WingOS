#pragma once
#include <klib/mouse.h>
#include <klib/process_message.h>
namespace sys {
    int32_t get_mouse_x(){
        ps2_device_request request = {0};
        request.device_target = 1;
        request.request_type = GET_MOUSE_POSITION;
        request.mouse_request_pos.get_x_value = true;
        process_message msg("ps2_device_service", (uint64_t)&request, sizeof (ps2_device_request));
        int32_t result = msg.read();
        if(result < 0){
            result *= -1;
        }

        return result;
    }
    int32_t get_mouse_y(){
        ps2_device_request request = {0};
        request.device_target = 1;
        request.request_type = GET_MOUSE_POSITION;
        request.mouse_request_pos.get_x_value = false;
        process_message msg("ps2_device_service", (uint64_t)&request, sizeof (ps2_device_request));
        int32_t result = msg.read();
        if(result < 0){
            result *= -1;
        }
        return result;

    }
}
