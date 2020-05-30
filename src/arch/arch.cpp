#include <arch/arch.h>
#include <kernel.h>
#include <com.h>
#include <arch/gdt.h>



extern "C" void kernel_start(stivale_struct *bootloader_data){

    com_initialize(COM_PORT::COM1);
   
    com_write_str("hello world");
    com_write_str("init gdt");
    setup_gdt();
    com_write_str("init gdt : OK");
    _start(bootloader_data);
}