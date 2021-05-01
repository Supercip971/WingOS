#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <kern/file.h>
#include <kern/process_message.h>
#include <kern/syscall.h>
#include <stdio.h>
#include <utils/sys/device_file_info.h>
namespace sys
{
    size_t get_framebuffer_width();
    size_t get_framebuffer_height();
    uintptr_t get_framebuffer_addr();
}
#endif // FRAMEBUFFER_H
