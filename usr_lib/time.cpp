#include <time.h>

#include <klib/kernel_time_util.h>
time_t time(time_t *second)
{
    if (second != nullptr)
    {
        return 0;
    }
    else
    {
        return sys::get_time_total_sec();
    }
}
