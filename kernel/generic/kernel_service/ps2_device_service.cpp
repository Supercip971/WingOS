#include <device/ps_keyboard.h>
#include <device/ps_mouse.h>
#include <kernel_service/ps2_device_service.h>
#include <logging.h>
#include <process.h>
general_mouse *mouse_target;
general_keyboard *keyb_target;
struct raw_request_data
{

    uint8_t raw_data[32];
} __attribute__((packed));

struct mouse_get_position
{
    bool get_x_value;
} __attribute__((packed));

struct mouse_get_button
{
    int mouse_button_type;
} __attribute__((packed));
struct get_keyboard_key_down
{
    bool unused;
} __attribute__((packed));

struct ps2_device_request
{
    uint8_t device_target; // for the moment 1 = mouse 2 = keyboard
    uint64_t request_type;

    union
    {
        raw_request_data data;
        mouse_get_position mouse_request_pos;
        mouse_get_button mouse_button_request;
        get_keyboard_key_down get_key_down;
    };
} __attribute__((packed));

enum mouse_request_type
{
    GET_MOUSE_POSITION = 0,
    GET_MOUSE_BUTTON = 1
};

enum keyboard_request_type
{

    GET_KEYBOARD_KEY = 0,
};

enum mouse_button_type
{
    GET_MOUSE_LEFT_CLICK = 0,
    GET_MOUSE_RIGHT_CLICK = 1,
    GET_MOUSE_MIDDLE_CLICK = 2
};

uint64_t mouse_handle(ps2_device_request *request)
{

    if (request->request_type == GET_MOUSE_POSITION)
    {
        if (request->mouse_request_pos.get_x_value == true)
        {
            return mouse_target->get_mouse_x();
        }
        return mouse_target->get_mouse_y();
    }
    else if (request->request_type == GET_MOUSE_BUTTON)
    {
        return mouse_target->get_mouse_button(request->mouse_button_request.mouse_button_type);
    }

    log("ps2 service", LOG_ERROR) << "mouse error request not handled : " << request->request_type;
    return -2;
}

uint64_t keyboard_handle(ps2_device_request *request)
{

    if (request->request_type == GET_KEYBOARD_KEY)
    {
        return keyb_target->get_last_keypress();
    }
    log("ps2 service", LOG_ERROR) << "keyboard error request not handled : " << request->request_type;
    return -2;
}

void ps2_device_service()
{

    log("ps2 service", LOG_INFO) << "loaded ps2 device service";

    uint64_t *mx = (uint64_t *)get_current_process_global_data(0x0, sizeof(uint64_t));
    uint64_t *my = (uint64_t *)get_current_process_global_data(0x0 + sizeof(uint64_t) * 1, sizeof(uint64_t));
    uint64_t *mclickl = (uint64_t *)get_current_process_global_data(0x0 + sizeof(uint64_t) * 2, sizeof(uint64_t));
    uint64_t *mclickr = (uint64_t *)get_current_process_global_data(0x0 + sizeof(uint64_t) * 3, sizeof(uint64_t));
    uint64_t *mclickm = (uint64_t *)get_current_process_global_data(0x0 + sizeof(uint64_t) * 4, sizeof(uint64_t));
    *mclickl = 0;
    *mclickr = 0;
    *mclickm = 0;
    *my = 0;
    *mx = 0;
    mouse_target = find_device<general_mouse>(device_type::MOUSE_DEVICE);
    mouse_target->set_ptr_to_update((uint32_t *)(mx), (uint32_t *)(my));
    while (true)
    {
        //  *mx = (uint64_t)ps_mouse::the()->get_mouse_x();
        //  *my = (uint64_t)ps_mouse::the()->get_mouse_y();
        *mclickl = (uint64_t)mouse_target->get_mouse_button(GET_MOUSE_LEFT_CLICK);
        *mclickm = (uint64_t)mouse_target->get_mouse_button(GET_MOUSE_MIDDLE_CLICK);
        *mclickr = (uint64_t)mouse_target->get_mouse_button(GET_MOUSE_RIGHT_CLICK);
        process_message *msg = read_message();

        if (msg != 0)
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
            msg->has_been_readed = true;
            msg->response = result;
        }
    }
}
