#include <arch/process.h>
#include <device/ps_mouse.h>
#include <kernel_service/ps2_device_service.h>
#include <loggging.h>
struct raw_request_data
{

    uint8_t raw_data[32];
};
struct mouse_get_position
{
    bool get_x_value;
};

struct ps2_device_request
{
    uint8_t device_target; // for the moment 1 = mouse 2 = keyboard
    uint64_t request_type;

    union
    {
        raw_request_data data;
        mouse_get_position mouse_request_pos;
    };
} __attribute__((packed));
enum mouse_request_type
{
    GET_MOUSE_POSITION = 0,
    GET_MOUSE_BUTTON = 1
};

uint64_t mouse_handle(ps2_device_request *request)
{

    if (request->request_type == GET_MOUSE_POSITION)
    {
        if (request->mouse_request_pos.get_x_value == true)
        {
            return ps_mouse::the()->get_mouse_x();
        }
        return ps_mouse::the()->get_mouse_y();
    }

    log("ps2 service", LOG_ERROR) << "error request not handled : " << request->request_type;
    return -2;
}

uint64_t keyboard_handle(ps2_device_request *request)
{

    log("ps2 service", LOG_ERROR) << "error keyboard is not implemented for the moment";

    return -2;
}
void ps2_device_service()
{
    set_on_request_service(true);

    log("ps2 service", LOG_INFO) << "loaded ps2 device service";

    while (true)
    {
        process_message *msg = read_message();
        if (msg == 0x0)
        {
        }
        else
        {

            ps2_device_request *request = (ps2_device_request *)msg->content_address;
            uint64_t result = -2;
            if (request->device_target == 1)
            {
                result = mouse_handle(request);
            }
            else if (request->device_target == 2)
            {
                result = keyboard_handle(request);
            }
            if (result == 0)
            {
                result = 1;
            }
            msg->has_been_readed = true;
            msg->response = result;

            on_request_service_update();
        }
    }
}
