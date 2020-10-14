#include <arch/arch.h>
#include <arch/mem/virtual.h>
#include <arch/process.h>
#include <kernel_service/graphic_buffer_service.h>
#include <loggging.h>
#include <stivale_struct.h>

void graphic_buffer_service()
{
    stivale_struct *hdr = &boot_loader_data_copy;
    log("graphic_buffer", LOG_INFO) << "loaded graphic buffer service";
    set_on_request_service(true);
    while (true)
    {
        process_message *msg = read_message();

        if(!msg){
            graphic_buffer_protocol *data = reinterpret_cast<graphic_buffer_protocol *>(msg->content_address);
            if (data->request == GET_CURRENT_BUFFER_ADDR)
            {
                log("graphic_buffer", LOG_INFO) << "get current buffer address" << hdr->framebuffer_addr;
                msg->response = get_mem_addr(hdr->framebuffer_addr);
                msg->has_been_readed = true;
            }
            else if (data->request == GET_SCREEN_SIZE)
            {
                if (data->data1 == 0)
                {
                    log("graphic_buffer", LOG_INFO) << "get current screen width" << hdr->framebuffer_width;
                    msg->response = hdr->framebuffer_width;
                    msg->has_been_readed = true;
                }
                else
                {
                    log("graphic_buffer", LOG_INFO) << "get current screen height" << hdr->framebuffer_height;
                    msg->response = hdr->framebuffer_height;
                    msg->has_been_readed = true;
                }
            }
            else
            {
                log("graphic_buffer", LOG_ERROR) << "not supported request id: " << data->request;
            }
            on_request_service_update();
        }
    }
}
