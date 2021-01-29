#include "unistd.h"
#include <klib/syscall.h>
unsigned int sleep(unsigned int sec)
{
    timespec time = {0, 0};
    time.tv_sec = sec;
    return sys::sys$nano_sleep(&time, nullptr);
}

suseconds_t usleep(suseconds_t sec)
{

    timespec time = {0, 0};
    time.tv_sec = 0;
    time.tv_nsec = sec * 1000;
    return sys::sys$nano_sleep(&time, nullptr);
}
