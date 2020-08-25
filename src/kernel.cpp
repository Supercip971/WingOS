#include <int_value.h>
#include <stivale.h>
#include <kernel.h>
#include <utility.h>
#include <com.h>
#include <device/pit.h>
#include <arch/process.h>
/*
    pour le moment tout ce qui est ici est un test
    for the moment everything here is for test
*/
void _start( stivale_struct *bootloader_data) {

    asm volatile("sti");
    PIT::the()->init_PIT();
    uint64_t current_screen_pos = 0;
    uint32_t d = 100;
    char buffer[32]; // counter
    uint32_t* dd = (uint32_t*)
            bootloader_data->framebuffer_addr;
    uint64_t update_tick = 0;
    uint64_t started_sec = 0;
    unlock_process();
    while(true){
        for(uint64_t i = 0; i < bootloader_data->framebuffer_width * bootloader_data->framebuffer_height; i++){

            dd[i] = d;
        }
        update_tick++;
        if(PIT::the()->passed_sec != started_sec){
            started_sec = PIT::the()->passed_sec;
            memzero(buffer, 32);

            kitoaT<uint64_t>(buffer, 'd',update_tick);
            com_write_strl("fps  : ");
            com_write_str(buffer);
            update_tick = 0;
        }




        d++;

    }
}
