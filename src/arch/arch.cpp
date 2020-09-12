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
#include <device/apic_timer.h>
#include <device/ata_driver.h>
#include <device/hpet.h>
#include <device/local_data.h>
#include <device/madt.h>
#include <device/mboot_module.h>
#include <device/pit.h>
#include <filesystem/echfs.h>
#include <filesystem/partition/base_partition.h>
#include <kernel.h>
#include <loggging.h>
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
    asm volatile("cli");
    com_initialize(COM_PORT::COM1);

    printf("bootloader addr %x \n", (uint64_t)bootloader_data);
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

    printf("init paging \n");
    init_virtual_memory(bootloader_data);
    printf("init paging : OK \n");

    printf("loading pic \n");
    pic_init();
    printf("loading pic : OK \n");
    acpi::the()->init((reinterpret_cast<stivale_struct *>(bootdat))->rsdp);

    acpi::the()->getFACP();

    madt::the()->init();

    apic::the()->init();

    smp::the()->init();

    // PIT::the()->init_PIT();
    apic_timer::the()->init();
    for (int i = 0; i < 16; i++)
    {

        apic::the()->set_redirect_irq(0, i, 1);
    }

    //hpet::the()->init_hpet();
    printf("set global data \n");
    set_current_data(get_current_data());
    printf("set global data : OK \n");
    //virt_map((uint64_t)bootloader_data, (uint64_t)bootloader_data, 0x3);

    //    bootloader_data = (stivale_struct *)get_mem_addr((uint64_t)bootloader_data);

    mboot_module::the()->init(bootloader_data);
    bootdat = ((uint64_t)&boot_loader_data_copy);
    printf(" frame buffer address %x \n", boot_loader_data_copy.framebuffer_addr);
    printf(" frame buffer address %x \n", ((stivale_struct *)bootdat)->framebuffer_addr);
    printf(" bootloader_data address %x \n", (uint64_t)&boot_loader_data_copy);
    printf(" bootloader_data address %x \n ", (uint64_t)bootdat);
    printf("init process \n");
    lock_process();
    init_multi_process(start_process);
    asm volatile("sti");
}
MBR_partition mbr_part; // TODO : add [new] and [delete]
echfs main_start_fs;
void start_process()
{
    printf(" frame buffer address 2 %x \n", ((stivale_struct *)bootdat)->framebuffer_addr);
    printf("init process OK \n");
    printf("testing with memory \n");
    uint8_t *m = (uint8_t *)malloc(sizeof(uint8_t) * 128); // just for testing
    for (uint64_t i = 0; i < 128; i++)
    {
        m[i] = i;
    }
    printf("loading with memory \n");
    ata_driver::the()->init();
    mbr_part = MBR_partition();

    mbr_part.init();
    main_start_fs = echfs();
    main_start_fs.init(mbr_part.get_partition_start(0), mbr_part.get_partition_length(0));

    printf("testing with memory 3 \n");
    _start((stivale_struct *)bootdat);
}
