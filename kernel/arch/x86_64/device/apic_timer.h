#ifndef APIC_TIMER_H
#define APIC_TIMER_H
#include <general_device.h>
class apic_timer : public interrupt_timer
{
public:
    apic_timer(){};
    void set_clock(uint32_t clock) override;
    void init();
    void turn_off() override;
    void turn_on() override;
    const char *get_name() const final { return "apic timer"; };
};

#endif // APIC_TIMER_H
