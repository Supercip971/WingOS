#pragma once
#include <stdint.h>



typedef long int time_t;
namespace sys {

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

    time_t get_time_total_sec();

}
