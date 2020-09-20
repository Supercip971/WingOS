#include <arch/process.h>
#include <kernel_service/print_service.h>

void print_service()
{
    log("console_out", LOG_INFO) << "loaded print service";

    while (true)
    {
        process_message *msg = read_message();
        if (msg == 0x0)
        {
        }
        else
        {
            asm("cli");
            log("console_out", LOG_INFO) << (char *)msg->content_address; // this is very insecure :(
            msg->has_been_readed = true;
            msg->entry_free_to_use = true;
            msg->response = 10; // 10 for yes

            asm("sti");
        }
    }
}
