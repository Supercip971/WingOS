#include <klib/mem_util.h>
#include <klib/kernel_util.h>
#include <klib/process_message.h>
namespace sys {

    void* service_malloc(size_t length){
        memory_service_protocol prot_data;
        prot_data.request_type = REQUEST_MALLOC;
        prot_data.length = length;
        prot_data.address = 0;
        sys::process_message msg = sys::process_message("memory_service", (uint64_t)&prot_data, sizeof (prot_data));
        uint64_t result = msg.read();
        return (void*) result;
    }

    void service_free(void* addr){
        memory_service_protocol prot_data;
        prot_data.request_type = REQUEST_FREE;
        prot_data.length = 0;
        prot_data.address = (uint64_t) addr;
        sys::process_message msg = sys::process_message("memory_service", (uint64_t)&prot_data, sizeof (prot_data));
        msg.read();
        return;
    }

    void* service_realloc(void* addr, size_t length){
        memory_service_protocol prot_data;
        prot_data.request_type = REQUEST_REALLOC;
        prot_data.length = length;
        prot_data.address = (uint64_t) addr;
        sys::process_message msg = sys::process_message("memory_service", (uint64_t)&prot_data, sizeof (prot_data));
        return (void*)msg.read();
    }
}
