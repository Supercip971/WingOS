#include <arch/arch.h>
#include <arch/gdt.h>
#include <arch/interrupt.h>
#include <arch/mem/liballoc.h>
#include <arch/mem/virtual.h>
#include <arch/msr_syscall.h>
#include <arch/pic.h>
#include <arch/process.h>
#include <arch/programm_launcher.h>
#include <arch/smp.h>
#include <arch/sse.h>
#include <com.h>
#include <device/acpi.h>
#include <device/apic.h>
#include <device/apic_timer.h>
#include <device/ata_driver.h>
#include <device/hpet.h>
#include <device/local_data.h>
#include <device/madt.h>
#include <device/mboot_module.h>
#include <device/pci.h>
#include <device/pit.h>
#include <device/rtc.h>
#include <filesystem/echfs.h>
#include <filesystem/partition/base_partition.h>
#include <kernel.h>
#include <kernel_service/kernel_service.h>
#include <loggging.h>

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
    init_sse();

    com_initialize(COM_PORT::COM1);
    memcpy(&boot_loader_data_copy, bootloader_data, sizeof(stivale_struct));

    setup_gdt((uint64_t)stack + (sizeof(char) * STACK_SIZE));

    init_idt();

    tss_init((uintptr_t)stack + sizeof(char) * STACK_SIZE);

    init_physical_memory(bootloader_data);
    init_vmm(bootloader_data);

    pic_init();
    RTC::the()->init();
    acpi::the()->init((reinterpret_cast<stivale_struct *>(bootdat))->rsdp);

    acpi::the()->getFACP();

    madt::the()->init();

    apic::the()->init();

    smp::the()->init();

    apic_timer::the()->init();
    for (int i = 0; i < 16; i++)
    {

        apic::the()->set_redirect_irq(0, i, 1);
    }

    set_current_data(get_current_data());

    mboot_module::the()->init(bootloader_data);
    bootdat = ((uint64_t)&boot_loader_data_copy);
    lock_process();
    init_multi_process(start_process);
    asm volatile("sti");
}
MBR_partition mbr_part; // TODO : add [new] and [delete]
echfs main_start_fs;
void start_process()
{
    load_kernel_service();
    uint8_t *m = (uint8_t *)malloc(sizeof(uint8_t) * 128); // just for testing
    for (uint64_t i = 0; i < 128; i++)
    {
        m[i] = i;
    }
    ata_driver::the()->init();
    mbr_part = MBR_partition();

    mbr_part.init();
    main_start_fs = echfs();
    main_start_fs.init(mbr_part.get_partition_start(0), mbr_part.get_partition_length(0));
    pci_system::the()->init();
    lock_process();
    launch_programm("init_fs/graphic_service.exe", &main_start_fs);
    launch_programm("init_fs/test2.exe", &main_start_fs);
    launch_programm("init_fs/test.exe", &main_start_fs);
    unlock_process();
    _start((stivale_struct *)bootdat);
}
