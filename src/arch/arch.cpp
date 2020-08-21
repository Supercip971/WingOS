#include <arch/arch.h>
#include <kernel.h>
#include <com.h>
#include <arch/gdt.h>
#include <arch/interrupt.h>
#include <arch/mem/virtual.h>
static char stack[4096] = {0};

__attribute__((section(".stivalehdr"), used))
struct stivale_header header = {
    .stack = (uintptr_t)stack + (sizeof(char)*4096),
    .flags = 0,
    .framebuffer_width = 0,
    .framebuffer_height = 0,
    .framebuffer_bpp = 0,
    .entry_point = 0
};

extern "C" void kernel_start(stivale_struct *bootloader_data){

    asm volatile("and rsp, -16");
    com_initialize(COM_PORT::COM1);
   
    com_write_str("hello world");
    com_write_str("init gdt");
    setup_gdt((uintptr_t)stack + (sizeof(char)*4096));
    com_write_str("init gdt : ✅");
    com_write_str("init idt");
    
    init_idt();
    com_write_str("init idt : ✅");

    com_write_str("init tss");
    tss_init((uintptr_t)stack + sizeof(char)*4096);
    com_write_str("init tss : OK");
    com_write_str("init paging");
    init_virtual_memory(bootloader_data);
    _start(bootloader_data);
}