#ifndef RTC_H
#define RTC_H
#include <stdint.h>
// rtc code is just here temporarly for APIC timer
class RTC
{
public:
    RTC();
    void init();
    uint8_t get_sec();
    static RTC *the();
};

#endif // RTC_H
