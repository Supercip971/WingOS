#include <klib/syscall.h>
#include <klib/process_message.h>
#include <klib/mem_util.h>
#include <klib/graphic_system.h>
#include <kgui/window.h>
#include <klib/raw_graphic.h>
#include <stdio.h>
int main(){
    sys::syscall(sys::NULL_SYSCALL, 12,2,3,4,5);
    uint64_t sample_data = 0;
    sys::process_message msg_to_send = sys::process_message("init_fs/test2.exe", (uint64_t)&sample_data,sizeof (uint64_t));
    uint64_t result = 0;
    const char* h = "hello world";
    sys::process_message helloworld = sys::process_message("console_out", (uint64_t)h, 13);
    printf("hello world, here we go again %s", "another string OWOWU ?");
    char* data= (char*)sys::service_malloc(1000);
    printf("g buffer addr : %x \n", sys::get_graphic_buffer_addr());
    for(int i= 0; i < 100; i++){
        data[i] = 0;
    }
    gui::window test_window("my window", 300,200);


    test_window.start();
    while (true) {
        result = msg_to_send.read();
        if(result != -2){
            msg_to_send = sys::process_message("init_fs/test2.exe", (uint64_t)&sample_data,sizeof (uint64_t));

            sys::syscall(sys::NULL_SYSCALL, 4,result,3,4,5);
        }

    }
    return 1;
}
