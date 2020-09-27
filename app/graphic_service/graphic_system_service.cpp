#include <klib/syscall.h>
#include <klib/process_message.h>
#include <klib/mem_util.h>
#include <klib/raw_graphic.h>
#include <stdio.h>


uint64_t real_gbuffer_addr = 0x0;
int main(){

    printf("graphic system service loading... \n");
    real_gbuffer_addr = sys::get_graphic_buffer_addr();
    printf("g buffer addr : %x \n", real_gbuffer_addr);
    while (true) {

    }
    return 1;
}
