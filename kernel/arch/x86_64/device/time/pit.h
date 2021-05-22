#pragma once
#include <device/general_device.h>
#include <stdint.h>
#define PIT_START_FREQUENCY 1193182
#define PIT_TARGET_FREQUECY 1000

class PIT : public interrupt_timer
{
    uint64_t passed_h = 0;
    uint64_t passed_min = 0;

public:
    uint64_t passed_sec = 0;
    uint64_t total_count = 0;
    int current_count = 0;
    void update();
    void init();
    void Pwait(uint16_t ms_count);

    virtual void set_clock(uint32_t clock){};
    virtual void turn_off(){};
    virtual void turn_on(){};
};
