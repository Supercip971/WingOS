#include <arch/arch.h>
#include <device/rtc.h>
#include <kernel.h>
#include <loggging.h>
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
    while (rtc_is_updating())
        ;
    outb(0x70, 0x0);
    return inb(0x71);
}
void RTC::init()
{
    log("rtc", LOG_DEBUG) << "loading rtc...";
    outb(0x70, 0x8A);
    outb(0x71, 0x20);
}
RTC *RTC::the()
{
    return &main_rtc;
}
