#include <klib/syscall.h>
#include <klib/process_message.h>
#include <klib/mem_util.h>
#include <klib/raw_graphic.h>
#include <stdio.h>
#include <klib/graphic_system.h>
#include <klib/process_message.h>
#include <klib/mem_util.h>
#define MAX_WINDOW 1024


// [!] BEFORE READING THIS CODE
// [!] EVERYTHING HERE WILL BE DIVIDED IN MULTIPLE FILE FOR THE MOMENT IT IS LIKE THAT
// [!] I WILL CLEAN UP EVERYTHING WHEN I WILL BE ABLE JUST TO CLEAR A WINDOW FROM AN APPLICATION
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
    pixel(uint32_t p){
        pix = p;
    };
    pixel(uint8_t vr, uint8_t vg, uint8_t vb, uint8_t va =  255){
        r = vr;
        g = vg;
        b = vb;
        a = va;
    }
};
struct raw_window_data{
    uint64_t wid;
    uint64_t pid;
    uint64_t px;
    uint64_t py;
    uint64_t width;
    uint64_t height;
    char* window_name;
    pixel* window_front_buffer;
    pixel* window_back_buffer;
    bool used;
};

pixel* front_buffer;
pixel* back_buffer;
uint64_t real_gbuffer_addr = 0x0;
uint64_t scr_width = 0;
uint64_t scr_height = 0;
uint64_t window_count = 0;
void draw_window(raw_window_data window, pixel* buffer){
    const uint64_t win_width = window.width;
    const uint64_t win_height = window.height;
    for(uint64_t x = 0; x < win_width; x++){
        for(uint64_t y = 0; y < win_height; y++){
            const uint64_t pos_f = (x+window.px) + (y+window.py) *scr_width;
            const uint64_t pos_t = (x) + (y) * scr_width;
            if(pos_f > scr_width * scr_height){
                return;
            }
            buffer[pos_f].pix = window.window_front_buffer[pos_t].pix;
        }
    }
}
raw_window_data* window_list;


void clear_buffer(pixel* buffer, uint64_t size, pixel value = {0,0,0,255}){
    const uint64_t msize = size / 2; // copy uint64_t
    const uint64_t rsize = size % 2;
    uint64_t* conv_buffer = (uint64_t*)buffer;
    const uint64_t v = (uint64_t)value.pix | ((uint64_t)value.pix << 32);
    for(uint64_t i = 0;i < msize; i++){
        conv_buffer[i] = v;
    }for(uint64_t i = msize*2; i < msize*2 + rsize; i++){
        buffer[i].pix = value.pix;
    }
}
uint64_t create_window(sys::graphic_system_service_protocol*request, uint64_t pid){
    printf("creating window for process %x", pid);
    for(int i = 0; i < MAX_WINDOW; i++){
        if(window_list[i].used == false){
            window_count++;
            window_list[i].used  = true;
            window_list[i].pid = pid;
            window_list[i].width = request->create_window_info.width;
            window_list[i].height = request->create_window_info.height;
            window_list[i].px = 0;
            window_list[i].py = 0;
            window_list[i].window_name = request->create_window_info.name;
            window_list[i].wid = i;
            window_list[i].window_front_buffer = (pixel*)sys::service_malloc(request->create_window_info.width * request->create_window_info.height* sizeof (pixel));
            window_list[i].window_back_buffer  = (pixel*)sys::service_malloc(request->create_window_info.width * request->create_window_info.height* sizeof (pixel));
            return i;
        }
    }

    return -1;
}
uint64_t interpret(sys::graphic_system_service_protocol* request, uint64_t pid){
    if(request->request_type == 0){
        printf("graphic error : request null type");
        return -2;
    }else if(request->request_type == sys::GRAPHIC_SYSTEM_REQUEST::CREATE_WINDOW){
        return create_window(request, pid);
    }
    printf("graphic error : request non implemented type");
    return -2;
}
void swap_buffer(pixel* buffer1, const pixel* buffer2, uint64_t buffer_length){
    uint64_t buffer_length_r64 = buffer_length / 2;
    uint64_t* to = (uint64_t*)buffer1;
    const uint64_t* from = (const uint64_t*)buffer2;
    for(uint64_t i = 0; i < buffer_length_r64; i++){
        to[i] = from[i];
    }
}
int main(){
    window_count = 0;
    printf("main graphic system service loading... \n");
    real_gbuffer_addr = sys::get_graphic_buffer_addr();
    front_buffer = (pixel*)real_gbuffer_addr;
    scr_width = sys::get_screen_width();
    scr_height = sys::get_screen_height();
    back_buffer = (pixel*)sys::service_malloc(scr_width*scr_height*sizeof (pixel));
    window_list = (raw_window_data*)sys::service_malloc(MAX_WINDOW * sizeof (raw_window_data));
    for(int i = 0; i < MAX_WINDOW; i++){
        window_list[i].used = false;
    }
    printf("g buffer addr   : %x \n", real_gbuffer_addr);
    printf("g buffer width  : %x \n",scr_width);
    printf("g buffer height : %x \n",scr_height);
    uint32_t soff = 0;
    while (true) {
        // read all message
        while(true){
            sys::raw_process_message* msg = sys::service_read_current_queue();
            if(msg != 0x0){
                sys::graphic_system_service_protocol* pr = (sys::graphic_system_service_protocol*)msg->content_address;

                msg->response = interpret(pr,  msg->from_pid);
                msg->has_been_readed = true;
            }
            break;
        }

        soff++;
        pixel r = pixel(soff);
        clear_buffer(back_buffer, scr_width * scr_height, soff);
        if(window_count != 0){
            for(int i = 0; i < window_count; i++){
                if(window_list[i].used == true){
                    swap_buffer(window_list[i].window_front_buffer, window_list[i].window_back_buffer, window_list[i].width*window_list[i].height);
                    draw_window(window_list[i],back_buffer);
                }
            }
        }
        swap_buffer(front_buffer, back_buffer, scr_width*scr_height);
    }
    return 1;
}
