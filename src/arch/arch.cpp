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

stivale_struct boot_loader_data_copy;

void start_process();

extern "C" void kernel_start(stivale_struct *bootloader_data)
{
    asm volatile("and rsp, -16");
    com_initialize(COM_PORT::COM1);

    com_write_reg("bootloader addr", (uint64_t)bootloader_data);
    memcpy(&boot_loader_data_copy, bootloader_data, sizeof(stivale_struct));

    printf("init gdt \n");
    setup_gdt((uint64_t)stack + (sizeof(char) * STACK_SIZE));
    printf("init gdt : ✅ \n");
    printf("init idt \n");

    init_idt();
    printf("init idt : ✅ \n");

    printf("init tss \n");
    tss_init((uintptr_t)stack + sizeof(char) * STACK_SIZE);
    printf("init tss : OK");
    // before paging
    acpi::the()->init(((stivale_struct *)bootdat)->rsdp);

    printf("init paging \n");
    init_virtual_memory(bootloader_data);
    printf("init paging : OK \n");

    printf("mapping \n");
    printf("mapping ok \n");

    PIT::the()->init_PIT();

    acpi::the()->getFACP();

    madt::the()->init();

    apic::the()->init();

    smp::the()->init();

    printf("loading pic \n");
    pic_init(); // load the pic after
    printf("loading pic : OK \n");

    printf("set global data \n");
    set_current_data(get_current_data());
    printf("set global data : OK \n");
    //virt_map((uint64_t)bootloader_data, (uint64_t)bootloader_data, 0x3);

    //    bootloader_data = (stivale_struct *)get_mem_addr((uint64_t)bootloader_data);

    bootdat = ((uint64_t)&boot_loader_data_copy);
    asm volatile("sti");
    printf(" frame buffer address %x \n", boot_loader_data_copy.framebuffer_addr);
    printf(" frame buffer address %x \n", ((stivale_struct *)bootdat)->framebuffer_addr);
    printf(" bootloader_data address %x \n", (uint64_t)&boot_loader_data_copy);
    printf(" bootloader_data address %x \n ", (uint64_t)bootdat);
    printf("init process \n");
    init_multi_process(start_process);
}
void start_process()
{
    printf(" frame buffer address 2 %x \n", ((stivale_struct *)bootdat)->framebuffer_addr);
    printf("init process OK \n");
    printf("testing with memory \n");
    uint8_t *m = (uint8_t *)malloc(sizeof(uint8_t) * 128); // just for testing
    printf("testing with memory 2 \n");
    for (uint64_t i = 0; i < 128; i++)
    {
        m[i] = i;
    }
    printf("testing with memory 3 \n");
    _start((stivale_struct *)bootdat);
}
