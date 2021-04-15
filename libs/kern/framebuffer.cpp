#include "framebuffer.h"
namespace sys {
sys::file fb_file = sys::file(FRAMEBUFFER_FILE_BUFFER);
inline void use_fb_file()
{
    if (!fb_file.is_openned())
    {
        fb_file.open(FRAMEBUFFER_FILE_BUFFER);
    }
}

framebuffer_buff_info get_info(){

    use_fb_file();
    framebuffer_buff_info buff;
    fb_file.seek(0);
    fb_file.read((uint8_t*)&buff, sizeof(framebuffer_buff_info) );
    return buff;
}
size_t get_framebuffer_width(){
    return get_info().width;
}
size_t get_framebuffer_height(){
    return get_info().height;
}
uintptr_t get_framebuffer_addr(){
    return get_info().addr;
}
}
