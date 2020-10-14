#include <arch/process.h>
#include <device/rtc.h>
#include <kernel_service/time_service.h>
#include <loggging.h>

enum TIME_SERVICE_REQUEST_CODE
{
    get_sec = 0,
    get_min = 1,
    get_hour = 2,
    get_day = 3,
    get_month = 4,
    get_year = 5,
    get_current_stime = 6,

};

struct time_service_request
{
    bool get; // for later we will add set()
    uint8_t index;
    uint32_t value;
} __attribute__((packed));

void time_service()
{
    log("time_service", LOG_DEBUG) << "loading time service";

    set_on_request_service(true);
    while (true)
    {
        process_message *msg = read_message();
        if (msg != 0)
        {
            time_service_request *tsr = (time_service_request *)msg->content_address;
            uint64_t result = 0;
            switch (tsr->index)
            {
            case get_sec:
                result = RTC::the()->get_sec();
                break;

            case get_min:
                result = RTC::the()->get_min();
                break;

            case get_hour:
                result = RTC::the()->get_hour();
                break;

            case get_month:
                result = RTC::the()->get_month();
                break;

            case get_year:
                result = RTC::the()->get_year();
                break;

            case get_current_stime:
                result = RTC::the()->get_total_sec();
                break;
            default:
                result = -3;
            }

            msg->response = result;
            msg->has_been_readed = true;
            on_request_service_update();
        }
        else
        {
        }
    }
}
