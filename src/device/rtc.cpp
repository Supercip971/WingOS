#include <arch/arch.h>
#include <device/rtc.h>
#include <kernel.h>
#include <logging.h>
RTC main_rtc;
RTC::RTC()
{
}

int rtc_is_updating()
{
    outb(0x70, 0x0A);
    return inb(0x71) & 0x80;
}

uint8_t RTC::get_sec()
{
    return read(0);
}

uint8_t RTC::get_min()
{
    return read(0x2);
}

uint8_t RTC::get_hour()
{
    return read(0x4);
}

uint8_t RTC::get_day()
{
    return read(0x7);
}

uint8_t RTC::get_month()
{
    return read(0x8);
}

uint8_t RTC::get_year()
{
    return read(0x9);
}

uint8_t RTC::read(int reg)
{
    while (rtc_is_updating())
        ;
    outb(0x70, reg);

    return inb(0x71);
}

uint64_t RTC::get_total_sec()
{
    uint64_t result = 0;
    result += (get_year() - 1) * 365 * 24 * 60 * 60;
    result += (get_month() - 1) * 12 * 24 * 60 * 60;
    result += (get_day() - 1) * 24 * 60 * 60;
    result += (get_hour() - 1) * 60 * 60;
    result += (get_min() - 1) * 60;
    result += (get_sec());
    return result;
}

void RTC::init()
{
    log("rtc", LOG_DEBUG) << "loading rtc...";

    log("rtc", LOG_INFO) << "stime : " << get_total_sec();
    log("rtc", LOG_INFO) << "year  :" << get_year();
    log("rtc", LOG_INFO) << "month :" << get_month();
    log("rtc", LOG_INFO) << "day   :" << get_day();
    log("rtc", LOG_INFO) << "hour  :" << get_hour();
    log("rtc", LOG_INFO) << "min   :" << get_min();
    log("rtc", LOG_INFO) << "sec   :" << get_sec();
}

RTC *RTC::the()
{
    return &main_rtc;
}
