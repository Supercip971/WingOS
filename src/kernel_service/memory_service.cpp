#include <arch/mem/liballoc.h>
#include <arch/process.h>
#include <kernel_service/memory_service.h>
#include <loggging.h>
void memory_service()
{
    log("memory_service", LOG_INFO) << "loaded memory service";
    set_on_request_service(true);
    while (true)
    {
        process_message *msg = read_message();
        if (msg == 0x0)
        {
        }
        else
        {
            memory_service_protocol *prot = (memory_service_protocol *)msg->content_address;
            if (prot->request_type == REQUEST_FREE)
            {
                free((void *)prot->address);
                msg->response = 1;
                msg->has_been_readed = true;
                //    msg->entry_free_to_use = true;
            }
            else if (prot->request_type == REQUEST_MALLOC)
            {
                msg->response = 1;
                msg->response = (uint64_t)malloc(prot->length);
                msg->has_been_readed = true;
            }
            else if (prot->request_type == REQUEST_REALLOC)
            {
                msg->response = (uint64_t)realloc((void *)prot->address, prot->length);
                msg->has_been_readed = true;
            }
            else
            {
                msg->has_been_readed = true;
                msg->response = 0;
                log("memory_service", LOG_ERROR) << "not valid request memory" << prot->request_type;
            }
            on_request_service_update();
        }
    }
}
