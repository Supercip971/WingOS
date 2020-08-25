#include <arch/arch.h>
#include <arch/gdt.h>
#include <arch/interrupt.h>
#include <arch/mem/liballoc.h>
#include <arch/mem/virtual.h>
#include <arch/process.h>
#include <com.h>
#include <kernel.h>
#include <stivale.h>
static char stack[4096] = {0};

__attribute__((section(".stivalehdr"), used))
stivale_header header = {.stack = (uintptr_t)stack + (sizeof(char) * 4096),
                         .flags = 1,
                         .framebuffer_width = 1600,
                         .framebuffer_height = 1200,
                         .framebuffer_bpp = 32,
                         .entry_point = 0};

void start_process();
uint64_t bootdat = 0;
extern "C" void kernel_start(stivale_struct *bootloader_data)
{
    asm volatile("and rsp, -16");
    com_initialize(COM_PORT::COM1);

    com_write_str("hello world");
    com_write_str("init gdt");
    setup_gdt((uint64_t)stack + (sizeof(char) * 4096));
    com_write_str("init gdt : ✅");
    com_write_str("init idt");

    init_idt();
    com_write_str("init idt : ✅");

    com_write_str("init tss");
    tss_init((uintptr_t)stack + sizeof(char) * 4096);
    com_write_str("init tss : OK");
    com_write_str("init paging");
    init_virtual_memory(bootloader_data);
    com_write_str("init paging : OK");

    com_write_str("mapping");
    set_paging_dir((uint64_t)pl4_table);
    com_write_str("mapping ok");

    bootdat = (uint64_t)bootloader_data;
    com_write_reg(" frame buffer address ", bootloader_data->framebuffer_addr);
    com_write_reg(" frame buffer address 2", ((stivale_struct *)bootdat)->framebuffer_addr);
    com_write_reg(" bootloader_data address ", (uint64_t)bootloader_data);
    com_write_reg(" bootloader_data address 2 ", (uint64_t)bootdat);
    com_write_str("init process");
    init_multi_process(start_process);
}
void start_process()
{

    com_write_reg(" frame buffer address 2", ((stivale_struct *)bootdat)->framebuffer_addr);
    com_write_str("init process OK");
    com_write_str("testing with memory");
    uint8_t *m = (uint8_t *)malloc(sizeof(uint8_t) * 128);
    com_write_str("testing with memory 2 ");
    for (uint64_t i = 0; i < 128; i++)
    {
        m[i] = i;
    }
    com_write_str("testing with memory 3 ");

    _start((stivale_struct *)bootdat);
}
