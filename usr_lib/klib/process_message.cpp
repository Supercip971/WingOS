#include <klib/process_message.h>
#include <klib/syscall.h>
namespace sys
{

    process_message::process_message()
    {
        loaded = false;
    }

    process_message::process_message(const char *to, uint64_t address_to_send, uint64_t data_length)
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

    uint64_t process_message::read()
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

    raw_process_message *service_read_current_queue()
    {
        return sys$read_message();
    }
} // namespace sys
