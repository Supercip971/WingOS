
#include <kern/file.h>
#include <kern/kernel_util.h>
#include <kern/process_message.h>
#include <kern/syscall.h>
#include <stdlib.h>
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
        stdout.write((const uint8_t *)raw_data, length);

        return 1;
    }

    uint64_t get_process_pid(const char *process_name)
    {
        process_request pr = {0};
        pr.type = GET_PROCESS_PID;
        memcpy((uint8_t *)pr.gpp.process_name, process_name, strlen(process_name) + 1);
        uint64_t result = sys::service_message("kernel_process_service", (uint64_t)&pr, sizeof(pr)).read();
        return result;
    }
    uint64_t get_current_pid()
    {
        return sys::sys$getpid();
    }

    void set_current_process_as_a_service(const char *service_name, bool is_request_only)
    {
        process_request pr = {0};
        pr.type = SET_CURRENT_PROCESS_AS_SERVICE;
        pr.scpas.is_ors = is_request_only;
        memcpy(pr.scpas.service_name, service_name, strlen(service_name) + 1);
        uint64_t result = sys::service_message("kernel_process_service", (uint64_t)&pr, sizeof(pr)).read();
    }

    void ksleep(uint64_t time)
    {
        process_request pr = {0};
        pr.type = PROCESS_SLEEP;
        pr.sleep_counter = time;
        uint64_t result = sys::service_message("kernel_process_service", (uint64_t)&pr, sizeof(pr)).read();
    }

    uint64_t start_programm(const char *path)
    {
        programm_exec_info info;
        info.argc = 0;
        info.argv = nullptr;
        info.env = nullptr;
        info.name = path;
        info.path = path;
        return sys::sys$exec(&info);
    }
} // namespace sys
