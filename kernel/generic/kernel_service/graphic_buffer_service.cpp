#include <arch.h>
#include <kernel_service/graphic_buffer_service.h>
#include <logging.h>
#include <process.h>
#include <stivale_struct.h>
#include <virtual.h>

void graphic_buffer_service()
{
    stivale_struct *hdr = &boot_loader_data_copy;
    log("graphic_buffer", LOG_INFO) << "loaded graphic buffer service";
    set_on_request_service(true);

    while (true)
    {
        process_message *msg = read_message();

        if (msg != 0)
        {
            set_on_request_service(false);
            graphic_buffer_protocol *data = reinterpret_cast<graphic_buffer_protocol *>(msg->content_address);
            if (data->request == GET_CURRENT_BUFFER_ADDR)
            {
                msg->response = get_mem_addr(hdr->framebuffer_addr);
                msg->has_been_readed = true;
            }
            else if (data->request == GET_SCREEN_SIZE)
            {
                if (data->data1 == 0)
                {
                    msg->response = hdr->framebuffer_width;
                    log("graphic_buffer", LOG_INFO) << "fb width " << hdr->framebuffer_width;
                }
                else
                {
                    log("graphic_buffer", LOG_INFO) << "fb height" << hdr->framebuffer_height;
                    msg->response = hdr->framebuffer_height;
                }
                msg->has_been_readed = true;
            }
            else
            {
                log("graphic_buffer", LOG_ERROR) << "not supported request id: " << (uint64_t)data->request;
            }
            set_on_request_service(true);
        }
        else if (msg == 0)
        {
            on_request_service_update();
        }
    }
}
