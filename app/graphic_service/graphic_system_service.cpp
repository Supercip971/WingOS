#include <klib/syscall.h>
#include <klib/process_message.h>
#include <klib/mem_util.h>
#include <klib/raw_graphic.h>
#include <stdio.h>
#include <klib/graphic_system.h>

struct pixel{
    union{
        struct {
            uint8_t r;
            uint8_t g;
            uint8_t b;
            uint8_t a;
        };
        uint32_t pix;
    };

};

class graphic_window{
public:
    uint64_t window_id;
    uint64_t window_process;
    pixel* front_buffer;
    pixel* back_buffer;
};
pixel* front_buffer;
pixel* back_buffer;
uint64_t real_gbuffer_addr = 0x0;
uint64_t scr_width = 0;
uint64_t scr_height = 0;
int main(){

    printf("main graphic system service loading... \n");
    real_gbuffer_addr = sys::get_graphic_buffer_addr();
    front_buffer = (pixel*)real_gbuffer_addr;
    scr_width = sys::get_screen_width();
    scr_height = sys::get_screen_height();
    printf("g buffer addr   : %x \n", real_gbuffer_addr);
    printf("g buffer width  : %x \n",scr_width);
    printf("g buffer height : %x \n",scr_height);
    uint32_t soff = 0;
    while (true) {
        soff++;
        for(uint32_t x = 0; x < scr_width*scr_height; x++){
            front_buffer[x].pix = x+soff;
        }
    }
    return 1;
}
