#include <klib/process_message.h>
#include <klib/syscall.h>
namespace sys
{

    service_message::service_message()
    {
        loaded = false;
    }

    service_message::service_message(const char *to, uint64_t address_to_send, uint64_t data_length)
    {
        for (int i = 0; i < 9999; i++)
        {
            source = sys$send_message(address_to_send, data_length, to);
            if (source != nullptr)
            {
                return;
            }
        }
    }

    uint64_t service_message::read()
    {
        while (true)
        {
            uint64_t response = 0;
            response = sys$message_response(source);
            if (response == -1)
            {
                return 0;
            }
            else if (response == -2)
            {
                continue;
            }
            else
            {
                return response;
            }
        }
    }
    process_message::process_message(){

        loaded = false;
    }
    process_message::process_message(uint64_t to_pid, uint64_t address_to_send, uint64_t data_length){
        for (int i = 0; i < 9999; i++)
        {
            source = sys$send_message_pid(address_to_send, data_length, to_pid);
            if (source != nullptr)
            {
                return;
            }
        }
    }

    uint64_t process_message::read(){
        while (true)
        {
            uint64_t response = 0;
            response = sys$message_response(source);
            if (response == -1)
            {
                return 0;
            }
            else if (response == -2)
            {
                continue;
            }
            else
            {
                return response;
            }
        }
    }

    raw_process_message *service_read_current_queue()
    {
        return sys$read_message();
    }
} // namespace sys
