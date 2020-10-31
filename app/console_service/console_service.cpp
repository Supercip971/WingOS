#include <klib/syscall.h>
#include <stdlib.h>
#include <klib/process_message.h>
#include <klib/mem_util.h>
#include <string.h>
#include <klib/graphic_system.h>
#include <kgui/window.h>
#include <klib/kernel_util.h>
#include <klib/raw_graphic.h>
#include <kgui/widget/button_widget.h>
#include <kgui/widget/rectangle_widget.h>
#include <stdio.h>
#include <klib/console.h>
struct console_per_process{
    char* data; // null terminated
    uint64_t length;
    uint64_t allocated_length;
    uint64_t pid;
    bool is_free_to_use = false;
};
console_per_process console_list[100];
void init_console(){
    for(int i = 0; i < 100; i++){
        console_list[i].data = nullptr;
        console_list[i].length = 0;
        console_list[i].is_free_to_use = true;
        console_list[i].pid = 0;
    }
}
void create_console(sys::raw_process_message* msg){
    for(int i = 0; i < 100; i++){
        if(console_list[i].is_free_to_use){
            printf("creating console for %x \n", msg->from_pid);
            console_list[i].data = (char*)malloc(100);
            memset(console_list[i].data, 0, 100);
            console_list[i].is_free_to_use = false;
            console_list[i].allocated_length= 100;
            console_list[i].length = 0;
            console_list[i].pid = msg->from_pid;
            return;
        }
    }
}
console_per_process* find(uint64_t pid){
    for(int i = 0; i < 100; i++){
        if(!console_list[i].is_free_to_use){
            if(console_list[i].pid == pid){
                return &console_list[i];
            }
        }
    }
    return nullptr;
}
void increase_console(console_per_process* p, uint64_t added_length){
    if(p->allocated_length < p->length + added_length){
        p->allocated_length += 512;
        p->data = (char*)realloc(p->data, p->allocated_length);

    }
    p->length += added_length;
}
void write(sys::raw_process_message*  msg){
    sys::console_service_request* pr = (sys::console_service_request*)msg->content_address;
    console_per_process* target = find(msg->from_pid);
    if(target == nullptr){
        create_console(msg);
        return write(msg);
    }
    uint64_t last_length = target->length;
    increase_console(target, strlen(pr->write.raw_data)+1);
    memcpy((char*)((uint64_t)target->data + last_length), pr->write.raw_data, strlen(pr->write.raw_data)+1);
    printf(target->data);
}
int main(){
    init_console();
    sys::set_current_process_as_a_service("console_service", true);
    while(true){
        sys::raw_process_message* msg = sys::service_read_current_queue();
        if(msg != 0x0){
            sys::console_service_request* pr = (sys::console_service_request*)msg->content_address;
            if(pr->request_type == sys::CONSOLE_READ) {
            }else if(pr->request_type == sys::CONSOLE_WRITE) {
                write(msg);
                msg->response = 1;
            }
            msg->has_been_readed = true;
        }else{

        }
    }


    return 0;
}
