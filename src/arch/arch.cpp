#include <arch/arch.h>
#include <arch/gdt.h>
#include <arch/interrupt.h>
#include <arch/mem/liballoc.h>
#include <arch/mem/virtual.h>
#include <arch/pic.h>
#include <arch/process.h>
#include <arch/smp.h>
#include <com.h>
#include <device/acpi.h>
#include <device/apic.h>
#include <device/local_data.h>
#include <device/madt.h>
#include <device/pit.h>
#include <kernel.h>
#include <stivale_struct.h>

static uint64_t bootdat;
static char stack[STACK_SIZE] = {0};

__attribute__((section(".stivalehdr"), used))
stivale_header header = {.stack = (uintptr_t)stack + (sizeof(char) * STACK_SIZE),
                         .flags = 1,
                         .framebuffer_width = 1600,
                         .framebuffer_height = 1200,
                         .framebuffer_bpp = 32,
                         .entry_point = 0};

void start_process();
extern "C" void kernel_start(stivale_struct *bootloader_data)
{
    asm volatile("and rsp, -16");
    com_initialize(COM_PORT::COM1);

    com_write_reg("bootloader addr", (uint64_t)bootloader_data);
    com_write_str("init gdt");
    setup_gdt((uint64_t)stack + (sizeof(char) * STACK_SIZE));
    com_write_str("init gdt : ✅");
    com_write_str("init idt");

    init_idt();
    com_write_str("init idt : ✅");

    com_write_str("init tss");
    tss_init((uintptr_t)stack + sizeof(char) * STACK_SIZE);
    com_write_str("init tss : OK");
    // before paging
    acpi::the()->init(((stivale_struct *)bootdat)->rsdp);

    com_write_str("init paging");
    init_virtual_memory(bootloader_data);
    com_write_str("init paging : OK");

    com_write_str("loading pic ");
    pic_init(); // load the pic first
    com_write_str("loading pic : OK ");

    com_write_str("mapping");
    set_paging_dir((uint64_t)pl4_table);
    com_write_str("mapping ok");
    PIT::the()->init_PIT();
    acpi::the()->getFACP();
    madt::the()->init();
    apic::the()->init();
    // smp is here but we doesn't use it for the moment
    //    smp::the()->init();
    com_write_str("set global data ");
    set_current_data(get_current_data());
    com_write_str("set global data : OK");
    asm("sti");
    bootdat = ((uint64_t)bootloader_data);
    bootloader_data = (stivale_struct *)((uint64_t)bootloader_data);
    com_write_reg(" frame buffer address ", (uint64_t)bootloader_data->framebuffer_addr);
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
    uint8_t *m = (uint8_t *)malloc(sizeof(uint8_t) * 128); // just for testing
    com_write_str("testing with memory 2 ");
    for (uint64_t i = 0; i < 128; i++)
    {
        m[i] = i;
    }
    com_write_str("testing with memory 3 ");
    _start((stivale_struct *)bootdat);
}
