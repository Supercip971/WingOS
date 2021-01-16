#include <device/apic.h>
#include <device/apic_timer.h>
#include <device/rtc.h>
#include <logging.h>
apic_timer main_apic_timer;

void apic_timer::init()
{
    add_device(this);
    log("apic timer", LOG_DEBUG) << "loading apic timer";
    apic::the()->write(timer_div, 0x3);

    uint8_t start_sec = RTC::the()->get_sec();
    while (start_sec == RTC::the()->get_sec())
        ;

    start_sec = RTC::the()->get_sec(); // wait til the start of a new sec

    apic::the()->write(timer_init_counter, 0xFFFFFFFF);
    while (start_sec == RTC::the()->get_sec())
        ;

    apic::the()->write(lvt_timer, 0x10000);

    uint64_t ticks = 0xFFFFFFFF - apic::the()->read(timer_current);
    ticks /= 1000; // 1 ms
    apic::the()->write(lvt_timer, 32 | 0x20000);
    apic::the()->write(timer_div, 0x3);
    apic::the()->write(timer_init_counter, ticks);
}

void apic_timer::set_clock(uint32_t clock)
{
    log("apic timer", LOG_WARNING) << __PRETTY_FUNCTION__ << "not implemented";
};
void apic_timer::turn_off()
{
    log("apic timer", LOG_WARNING) << __PRETTY_FUNCTION__ << "not implemented";
};
void apic_timer::turn_on()
{
    log("apic timer", LOG_WARNING) << __PRETTY_FUNCTION__ << "not implemented";
}
