#include <kernel.h>
#include <kernel_service/kernel_buffer_service.h>
#include <kernel_service/kernel_service.h>
#include <liballoc.h>
#include <process.h>
#include <programm_launcher.h>
#include <utility.h>
uint64_t process_buffer_read(process_buffer_request *request)
{
    if (request->gpb.get_buffer_length)
    {
        uint64_t target = upid_to_kpid(request->gpb.pid_target);
        if (target != (uint64_t)-1 && request->gpb.buffer_type <= 3)
        {
            process_buffer *buf = &process_array[target].pr_buff[request->gpb.buffer_type];
            return buf->length;
        }
        else
        {
            return -1;
        }
    }
    else
    {
        uint64_t target = upid_to_kpid(request->gpb.pid_target);

        if (target != (uint64_t)-1 && request->gpb.buffer_type <= 3)
        {
            process_buffer *buf = &(process_array[target].pr_buff[request->gpb.buffer_type]);
            if (request->gpb.where > buf->length)
            {
                return -1;
            }

            uint64_t min = request->gpb.length_to_read;
            if (min + request->gpb.where > buf->length)
            {
                min = buf->length - request->gpb.where;
            }
            memcpy(request->gpb.target_buffer, buf->data + request->gpb.where, min);
            return min;
        }
        else
        {
            return -1;
        }
    }
}
void kernel_process_buffer_service()
{

    log("kernel buffer service", LOG_INFO) << "loaded kernel buffer service";
    set_on_request_service(true);

    while (true)
    {
        process_message *msg = read_message();

        if (msg != 0)
        {
            set_on_request_service(false);
            process_buffer_request *prot = (process_buffer_request *)msg->content_address;
            // don't like switch >:(
            // they have some "optimization" but in this case meh

            if (prot->type == GET_PROCESS_BUFFER)
            {
                msg->response = process_buffer_read(prot);
            }

            else if (prot->type == OUT_PROCESS_BUFFER)
            {
                uint64_t target = upid_to_kpid(prot->opb.pid_target);
                if (target == (uint64_t)-1)
                {
                    log("kernel buffer service", LOG_WARNING) << "upid" << prot->opb.pid_target << "is invalid";
                }
                else
                {
                    msg->response = add_process_buffer(
                        &process_array[upid_to_kpid(prot->opb.pid_target)].pr_buff[prot->opb.buffer_type],
                        prot->opb.length,
                        prot->opb.output_data);
                }
            }

            else
            {
                log("kernel buffer service", LOG_ERROR) << "invalid request id : " << (uint64_t)prot->type << "from process" << msg->from_pid;
                msg->response = -1;
            }
            msg->has_been_readed = true;
            set_on_request_service(true);
        }
        else if (msg == 0)
        {
        }
    }
}
