#include <klib/kernel_util.h>
#include <klib/process_message.h>
#include <string.h>
namespace sys
{

    int write_console(const char *raw_data, int length)
    {
        if (length < 0)
        {
            return -2;
        }
        if (raw_data == nullptr)
        {
            return -1;
        }
        sys::process_message("console_out", (uint64_t)raw_data, length).read();
        return 1;
    }

    uint64_t get_process_pid(const char *process_name)
    {
        process_request pr = {0};
        pr.type = GET_PROCESS_PID;
        memcpy(pr.gpp.process_name, process_name, strlen(process_name) + 1);
        uint64_t result = sys::process_message("kernel_process_service", (uint64_t)&pr, sizeof(pr)).read();
        return result;
    }

    void set_current_process_as_a_service(const char *service_name, bool is_request_only)
    {
        process_request pr = {0};
        pr.type = SET_CURRENT_PROCESS_AS_SERVICE;
        pr.scpas.is_ors = is_request_only;
        memcpy(pr.scpas.service_name, service_name, strlen(service_name) + 1);
        uint64_t result = sys::process_message("kernel_process_service", (uint64_t)&pr, sizeof(pr)).read();
    }
} // namespace sys
