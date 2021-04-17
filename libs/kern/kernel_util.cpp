
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
        printf("error: %s removed for the moment \n", __PRETTY_FUNCTION__);
        return 0;
    }
    uint64_t get_current_pid()
    {
        return sys::sys$getpid();
    }

    void set_current_process_as_a_service(const char *service_name, bool is_request_only)
    {
        printf("error: %s removed for the moment \n", __PRETTY_FUNCTION__);
    }

    void ksleep(uint64_t time)
    {
        printf("error: %s removed for the moment \n", __PRETTY_FUNCTION__);
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
