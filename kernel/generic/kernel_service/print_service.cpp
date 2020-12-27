#include <arch.h>
#include <com.h>
#include <kernel_service/print_service.h>
#include <process.h>

extern lock_type locker_print;
void print_service()
{
    log("console_out", LOG_INFO) << "loaded print service";
    set_on_request_service(true);
    while (true)
    {

        set_on_request_service(true);
        process_message *msg = read_message();

        if (msg != 0)
        {

            uint64_t target = upid_to_kpid(msg->from_pid);

            process_buffer *buf = &process_array[target].pr_buff[STDOUT];
            add_process_buffer(buf, msg->content_length, (uint8_t *)msg->content_address);

            while (locker_print.data != 0)
            {
            }
            echo_out((char *)msg->content_address, msg->content_length);
            msg->has_been_readed = true;
            msg->response = 10; // 10 for yes
        }
        else if (msg == 0)
        {

            on_request_service_update();
        }
    }
}
