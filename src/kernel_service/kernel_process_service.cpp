#include <kernel_service/kernel_process_service.h>
#include <kernel_service/kernel_service.h>

#include <arch/mem/memory_manager.h>
#include <arch/process.h>

void kernel_process_service()
{

    log("kernel_process_service", LOG_INFO) << "loaded kernel_process_service service";

    while (true)
    {
        process_message *msg = read_message();

        if (msg != 0)
        {
            process_request *prot = (process_request *)msg->content_address;
            switch (prot->type)
            {
            case GET_PROCESS_PID:
                msg->response = get_pid_from_process_name(prot->gpp.process_name);
                break;
            case SET_CURRENT_PROCESS_AS_SERVICE:
                log("kernel_process_service", LOG_INFO) << "SET_CURRENT_PROCESS_AS_SERVICE";
                rename_process(prot->scpas.service_name, msg->from_pid);
                set_on_request_service(prot->scpas.is_ors, msg->to_pid);
                msg->response = 1;
                break;
            default:
                log("kernel_process_service", LOG_ERROR) << "invalid request id : " << prot->type;
                msg->response = -2;
                break;
            }
            msg->has_been_readed = true;
        }
        else if (msg == 0)
        {
        }
    }
}
