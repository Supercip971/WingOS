#include <klib/syscall.h>
#include <klib/process_message.h>
#include <klib/mem_util.h>
#include <klib/graphic_system.h>
#include <kgui/window.h>
#include <klib/kernel_util.h>
#include <feather_language_lib/feather.h>
#include <klib/raw_graphic.h>
#include <kgui/widget/button_widget.h>
#include <kgui/widget/rectangle_widget.h>
#include <stdio.h>
#include <stdlib.h>
void click(uint64_t t){
    printf("%x \n", t);
}
int main(){
    char* data= (char*)malloc(1000);
    printf("g buffer addr : %x \n", sys::get_graphic_buffer_addr());
    for(int i= 0; i < 100; i++){
        data[i] = 0;
    }
    sys::write_kconsole("JJJJJJJJJJJJJJ", 10);
    gui::window test_window("my window", 300,200);
    gui::button_widget button(10,30,70,20, "a button");

    button.set_click_callback(click);
    test_window.add_widget((gui::widget*)&button);

    return test_window.start();
    return 0;
}
