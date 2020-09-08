#ifndef APIC_TIMER_H
#define APIC_TIMER_H

class apic_timer
{
public:
    apic_timer();
    void init();
    static apic_timer *the();
    void update();
};

#endif // APIC_TIMER_H
