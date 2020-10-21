
#include <arch/mem/memory_manager.h>
#include <arch/process.h>
#include <kernel_service/memory_service.h>
#include <logging.h>
void memory_service()
{
    log("memory_service", LOG_INFO) << "loaded memory service";
    set_on_request_service(true);
    while (true)
    {
        process_message *msg = read_message();

        if (msg != 0)
        {
            memory_service_protocol *prot = (memory_service_protocol *)msg->content_address;
            if (prot->request_type == REQUEST_FREE)
            {
                free((void *)prot->address);
                msg->response = 1;
                msg->has_been_readed = true;
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
            else if (prot->request_type == REQUEST_PMM_MALLOC)
            {
                log("memory_service", LOG_INFO) << "pmm alloc" << prot->length;

                msg->response = get_mem_addr((uint64_t)pmm_alloc_fast(prot->length));
                msg->has_been_readed = true;
            }
            else if (prot->request_type == REQUEST_PMM_FREE)
            {
                log("memory_service", LOG_INFO) << "pmm free" << prot->length;
                msg->response = (uint64_t)1;
                pmm_free((void *)prot->address, prot->length);
                msg->has_been_readed = true;
            }
            else
            {
                msg->has_been_readed = true;
                msg->response = 0;
                log("memory_service", LOG_ERROR) << "not valid request memory" << (uint64_t)prot->request_type;
            }
        }
        else if (msg == 0)
        {

            on_request_service_update();
        }
    }
}
