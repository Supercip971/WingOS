#include <klib/syscall.h>
#include <klib/process_message.h>
#include <klib/mem_util.h>
#include <klib/graphic_system.h>
#include <kgui/window.h>
#include <klib/raw_graphic.h>
#include <kgui/widget/rectangle_widget.h>
#include <stdio.h>
int main(){
    char* data= (char*)sys::service_malloc(1000);
    printf("g buffer addr : %x \n", sys::get_graphic_buffer_addr());
    for(int i= 0; i < 100; i++){
        data[i] = 0;
    }
    gui::window test_window("my window", 300,200);
    gui::rectangle_widget rec_widget(10,10,40,50, sys::pixel(64,0,0));
    test_window.add_widget((gui::widget*)&rec_widget);
    return test_window.start();
}
