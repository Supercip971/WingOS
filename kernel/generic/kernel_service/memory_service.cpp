
#include <kernel_service/memory_service.h>
#include <liballoc.h>
#include <logging.h>
#include <process.h>

void memory_service()
{
    log("memory_service", LOG_INFO) << "loaded memory service";
    set_on_request_service(true);

    while (true)
    {
        process_message *msg = read_message();

        if (msg != 0)
        {
            set_on_request_service(false);
            memory_service_protocol *prot = (memory_service_protocol *)msg->content_address;

            if (prot->request_type == REQUEST_PMM_MALLOC)
            {

                msg->response = (uint64_t)(pmm_alloc(prot->length));
                msg->has_been_readed = true;
            }
            else if (prot->request_type == REQUEST_PMM_FREE)
            {
                msg->response = (uint64_t)1;
                pmm_free((void *)((prot->address)), prot->length);
                msg->has_been_readed = true;
            }
            else
            {
                msg->response = 0;
                msg->has_been_readed = true;
                log("memory_service", LOG_ERROR) << "not valid request memory" << (uint64_t)prot->request_type;
                log("memory_service", LOG_ERROR) << "with process" << (uint64_t)msg->from_pid << " | " << process_array[upid_to_kpid(msg->from_pid)].process_name;
                process_array[upid_to_kpid(msg->from_pid)].process_backtrace.dump_backtrace();
            }

            set_on_request_service(true);
        }
        else if (msg == 0)
        {
        }
    }
}
