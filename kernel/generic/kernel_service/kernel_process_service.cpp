#include <kernel.h>
#include <kernel_service/kernel_process_service.h>
#include <kernel_service/kernel_service.h>
#include <process.h>
#include <programm_launcher.h>
#include <utility.h>
#include <utils/liballoc.h>
void kernel_process_service()
{

    log("kernel_process_service", LOG_INFO) << "loaded kernel_process_service service";
    while (true)
    {
        process_message *msg = read_message();

        if (msg != 0)
        {
            process_request *prot = (process_request *)msg->content_address;
            // don't like switch >:(
            // they have some "optimization" but in this case meh
            if (prot->type == SET_CURRENT_PROCESS_AS_SERVICE)
            {
                log("kernel_process_service", LOG_INFO) << "SET_CURRENT_PROCESS_AS_SERVICE";
                rename_process(prot->scpas.service_name, msg->from_pid);
                set_on_request_service(prot->scpas.is_ors, msg->to_pid);
                msg->response = 1;
            }
            else if (prot->type == PROCESS_SLEEP)
            {

                sleep(prot->sleep_counter, msg->from_pid);
                msg->response = 1;
            }
            else if (prot->type == LAUNCH_PROGRAMM)
            {
                msg->response = launch_programm(prot->lnp.path, main_fs_system::the()->main_fs());
            }
            else if (prot->type == GET_CURRENT_PID)
            {
                msg->response = msg->from_pid;
            }
            else
            {
                log("kernel_process_service", LOG_ERROR) << "invalid request id : " << (uint64_t)prot->type << "from process" << msg->from_pid;
                msg->response = -1;
            }
            msg->has_been_readed = true;
        }
        else if (msg == 0)
        {
        }
    }
}
