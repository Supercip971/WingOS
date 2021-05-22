#ifndef RTC_H
#define RTC_H
#include <stdint.h>
class RTC
{
public:
    void init() const; // wait a const init function ? is this legal ?
    uint8_t read(int reg) const;
    uint8_t get_sec() const;
    uint8_t get_min() const;
    uint8_t get_hour() const;

    uint8_t get_day() const;

    uint8_t get_month() const;

    uint8_t get_year() const;
    uint64_t get_total_sec() const;
    static const RTC *the();
};

#endif // RTC_H
