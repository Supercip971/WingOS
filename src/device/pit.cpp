#include <device/pit.h>
#include <kernel.h>
#include <arch/arch.h>
#include <com.h>
#pragma GCC optimize ("-O0")
PIT global_PIT ;

void PIT::init_PIT(){
    *this = PIT();
    com_write_str("loading PIT");
    uint16_t divisor = PIT_START_FREQUENCY / PIT_TARGET_FREQUECY; // to do : make this more portable

    outb(0x43, 0x36);

    uint8_t l = (uint8_t)(divisor & 0xFF);
    wait();
    outb(0x40, l);
    wait();
    uint8_t h = (uint8_t)( (divisor>>8) & 0xFF );
    outb(0x40, h);
    com_write_str("loaded PIT");

}

void PIT::update(){
    total_count ++;
    current_count ++;
    if(current_count > 1000){
        com_write_str("sec");
        current_count = 0;
        passed_sec += 1;
    }
}
bool loaded = false;

 PIT* PIT::the(){
    return &global_PIT;
}
