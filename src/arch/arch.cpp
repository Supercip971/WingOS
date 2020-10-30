#include <arch/arch.h>
#include <arch/gdt.h>
#include <arch/interrupt.h>
#include <arch/mem/liballoc.h>
#include <arch/mem/virtual.h>
//#include <arch/msr_syscall.h> sorry this file will exist one day
#include <arch/mem/memory_manager.h>
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
#include <device/ps_mouse.h>
#include <device/rtc.h>
#include <filesystem/echfs.h>
#include <filesystem/file_system.h>
#include <filesystem/partition/base_partition.h>
#include <kernel.h>
#include <kernel_service/kernel_service.h>
#include <logging.h>

__attribute__((section(".stivalehdr"), used))
stivale_header header = {.stack = (uintptr_t)stack + (sizeof(char) * STACK_SIZE),
                         .flags = 1,
                         .framebuffer_width = 1440,
                         .framebuffer_height = 900,
                         .framebuffer_bpp = 32,
                         .entry_point = 0};

stivale_struct boot_loader_data_copy;

void start_process();

//  ____    __    ____  __  .__   __.   _______      ______        _______.
//  \   \  /  \  /   / |  | |  \ |  |  /  _____|    /  __  \      /       |
//   \   \/    \/   /  |  | |   \|  | |  |  __     |  |  |  |    |   (----`
//    \            /   |  | |  . `  | |  | |_ |    |  |  |  |     \   \
//     \    /\    /    |  | |  |\   | |  |__| |    |  `--'  | .----)   |
//      \__/  \__/     |__| |__| \__|  \______|     \______/  |_______/
//

extern "C" void kernel_start(stivale_struct *bootloader_data)
{
    asm volatile("and rsp, -16");
    asm volatile("cli");
    init_sse();

    com_initialize(COM_PORT::COM1);
    log("COM1", LOG_INFO) << "com port 1 loaded";

    memcpy(&boot_loader_data_copy, bootloader_data, sizeof(stivale_struct));

    setup_gdt((uint64_t)stack + (sizeof(char) * STACK_SIZE));

    init_idt();
    PIT::the()->init_PIT();
    tss_init((uintptr_t)stack + sizeof(char) * STACK_SIZE);

    init_physical_memory(bootloader_data);
    init_vmm(bootloader_data);

    ps_mouse::the()->init();
    pic_init();
    log("memory manager", LOG_INFO) << "start test";
    void *last = nullptr;
    for (int i = 64; i < 100; i++)
    {
        log("memory manager", LOG_INFO) << "for" << i << "id : " << (uint64_t)last;
        if (i % 3 == 0)
        {
            free(last);
        }
        last = malloc(i);
    }
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

    set_current_data(get_current_cpu());

    mboot_module::the()->init(bootloader_data);
    bootdat = ((uint64_t)&boot_loader_data_copy);

    lock_process();
    printf("\n\n");
    printf(":::       ::: ::::::::::: ::::    :::  ::::::::         ::::::::   ::::::::\n");
    printf(":+:       :+:     :+:     :+:+:   :+: :+:    :+:       :+:    :+: :+:    :+:\n");
    printf("+:+       +:+     +:+     :+:+:+  +:+ +:+              +:+    +:+ +:+        \n");
    printf("+#+  +:+  +#+     +#+     +#+ +:+ +#+ :#:              +#+    +:+ +#++:++#++ \n");
    printf("+#+ +#+#+ +#+     +#+     +#+  +#+#+# +#+   +#+#       +#+    +#+        +#+ \n");
    printf(" #+#+# #+#+#      #+#     #+#   #+#+# #+#    #+#       #+#    #+# #+#    #+# \n");
    printf("  ###   ###   ########### ###    ####  ########         ########   ########  \n");

    printf("\n");
    init_multi_process(start_process);
    asm volatile("sti");
    asm("hlt");
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

    main_fs_system::the()->init_file_system();

    pci_system::the()->init();
    //   launch_programm("init_fs/memory_service.exe", main_fs_system::the()->main_fs());
    launch_programm("init_fs/console_service.exe", main_fs_system::the()->main_fs());

    launch_programm("init_fs/graphic_service.exe", main_fs_system::the()->main_fs());
    launch_programm("init_fs/test2.exe", main_fs_system::the()->main_fs());
    launch_programm("init_fs/test.exe", main_fs_system::the()->main_fs());
    launch_programm("init_fs/test.exe", main_fs_system::the()->main_fs());

    _start((stivale_struct *)bootdat);
}
