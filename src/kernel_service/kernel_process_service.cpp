#include <kernel_service/kernel_process_service.h>
#include <kernel_service/kernel_service.h>

#include <arch/mem/liballoc.h>
#include <arch/process.h>

void kernel_process_service()
{

    log("kernel_process_service", LOG_INFO) << "loaded kernel_process_service service";
    set_on_request_service(true);
    while (true)
    {
        process_message *msg = read_message();

        if(!msg){
            kernel_process_service_request *prot = (kernel_process_service_request *)msg->content_address;
            if (msg->content_length < 4096)
            {
                msg->response = get_pid_from_process_name((char *)msg->content_address);
            }
            msg->has_been_readed = true;
            on_request_service_update();
        }
    }
}
