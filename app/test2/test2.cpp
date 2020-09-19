#include <klib/syscall.h>
#include <klib/process_message.h>

int main(){
    sys::syscall(0, 1,34,3,4,5);
    while (true) {
        sys::raw_process_message* msg = sys::service_read_current_queue();
        if(msg != 0x0){
            msg->has_been_readed = true;
            msg->response = 11;
            sys::syscall(0, 0,1,0,0,0);
        }


    }
    return 0;
}
