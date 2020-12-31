#include <klib/kernel_util.h>
#include <klib/mem_util.h>
#include <klib/process_message.h>
#include <string.h>
namespace sys
{

    void *service_pmm_malloc(size_t length)
    {
        memory_service_protocol prot_data;
        prot_data.request_type = REQUEST_PMM_MALLOC;
        prot_data.length = length;
        prot_data.address = 0;
        sys::service_message msg = sys::service_message("memory_service", (uint64_t)&prot_data, sizeof(prot_data));
        uint64_t result = msg.read();
        return (void *)result;
    }
    void service_pmm_free(void *addr, size_t length)
    {
        memory_service_protocol prot_data;
        prot_data.request_type = REQUEST_PMM_FREE;
        prot_data.length = length;
        prot_data.address = (uint64_t)addr;
        sys::service_message msg = sys::service_message("memory_service", (uint64_t)&prot_data, sizeof(prot_data));
    }
} // namespace sys
