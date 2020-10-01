#ifndef RTC_H
#define RTC_H
#include <stdint.h>
// rtc code is just here temporarly for APIC timer
class RTC
{
public:
    RTC();
    void init();
    uint8_t read(int reg);
    uint8_t get_sec();
    uint8_t get_min();
    uint8_t get_hour();

    uint8_t get_day();

    uint8_t get_month();

    uint8_t get_year();
    uint64_t get_total_sec();
    static RTC *the();
};

#endif // RTC_H
