#include <arch/arch.h>
#include <arch/process.h>
#include <kernel_service/graphic_buffer_service.h>
#include <loggging.h>
#include <stivale_struct.h>
void graphic_buffer_service()
{
    stivale_struct *hdr = reinterpret_cast<stivale_struct *>(bootdat);
    log("graphic_buffer", LOG_INFO) << "loaded graphic buffer service";
    set_on_request_service(true);
    while (true)
    {
        process_message *msg = read_message();
        if (msg == 0x0)
        {
        }
        else
        {
            graphic_buffer_protocol *data = (graphic_buffer_protocol *)msg->content_address;
            if (data->request == GET_CURRENT_BUFFER_ADDR)
            {
                msg->response = hdr->framebuffer_addr;
            }
            msg->has_been_readed = true;
            on_request_service_update();
        }
    }
}
