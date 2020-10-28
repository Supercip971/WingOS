#pragma once
#include <int_value.h>

#define PIT_START_FREQUENCY 1193180
#define PIT_TARGET_FREQUECY 1000

class PIT
{
    uint64_t passed_h = 0;
    uint64_t passed_min = 0; // these are for later purpose
public:
    uint64_t passed_sec = 0;
    uint64_t total_count = 0;
    int current_count = 0; // from 0 to PIT_TARGET_FREQUECY oh and i hate c++
                           // getteur and setteur as it is so bad for optimization
                           // (or compile time) so nyeh :^( it is public value
    void update();
    void init_PIT();
    void Pwait(uint16_t ms_count);
    static PIT *the(); // get the global PIT
};
