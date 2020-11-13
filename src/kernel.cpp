#include <arch/mem/physical.h>
#include <arch/process.h>
#include <com.h>
#include <device/pit.h>
#include <device/rtc.h>
#include <int_value.h>
#include <kernel.h>
#include <logging.h>
#include <stdint.h>
#include <stivale_struct.h>
#include <utility.h>
/*
    pour le moment tout ce qui est ici est un test
    for the moment everything here is for test
*/
void _start(stivale_struct *bootloader_data)
{
    log("kernel", LOG_INFO) << "====                ====";
    log("kernel", LOG_INFO) << "==== KERNEL STARTED ====";
    log("kernel", LOG_INFO) << "====                ====";

    log("kernel", LOG_INFO) << "kernel started with " << get_total_memory() << "memory available";
    log("kernel", LOG_INFO) << "memory used : " << get_used_memory();
    while (true)
    {
        yield();
    }
}
