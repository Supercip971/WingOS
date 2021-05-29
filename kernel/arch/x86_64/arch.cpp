#include <arch.h>
#include <device/acpi.h>
#include <device/apic.h>
#include <device/debug/com.h>
#include <device/disk/ata_driver.h>
#include <device/graphic.h>
#include <device/local_data.h>
#include <device/madt.h>
#include <device/mboot_module.h>
#include <device/pci.h>
#include <device/ps_keyboard.h>
#include <device/ps_mouse.h>
#include <device/time/apic_timer.h>
#include <device/time/hpet.h>
#include <device/time/pit.h>
#include <device/time/rtc.h>
#include <filesystem/echfs.h>
#include <filesystem/file_system.h>
#include <filesystem/partition/base_partition.h>
#include <gdt.h>
#include <interrupt.h>
#include <kernel.h>
#include <logging.h>
#include <mem/virtual.h>
#include <pic.h>
#include <proc/process.h>
#include <programm_launcher.h>
#include <smp.h>
#include <sse.h>
#include <stddef.h>
#include <utility.h>
#include <utils/attribute.h>
#include <utils/memory/liballoc.h>
#include <utils/sys/config.h>

struct stackframe
{
    stackframe *rbp;
    uint64_t rip;
} __attribute__((packed));

void dump_stackframe(void *rbp)
{

    stackframe *frame = reinterpret_cast<stackframe *>(rbp);
    int size = 0;
    while (frame && size++ < 20)
    {
        log("stackframe", LOG_INFO, "{}", frame->rip);
        frame = frame->rbp;
    }
}

char stack[STACK_SIZE * 4] __attribute__((aligned(16))) = {0};
static uintptr_t bootdat = 0;
struct stivale2_header_tag_framebuffer framebuffer_hdr_tag = {
    .tag = {
        .identifier = STIVALE2_HEADER_TAG_FRAMEBUFFER_ID,
        .next = 0},
    .framebuffer_width = 1440,
    .framebuffer_height = 900,
    .framebuffer_bpp = 32};

__attribute__((section(".stivale2hdr"), used)) struct stivale2_header stivale_hdr = {
    .entry_point = 0,
    .stack = (uintptr_t)stack + sizeof(stack),
    .flags = 0b1,
    .tags = (uintptr_t)&framebuffer_hdr_tag};

struct stivale2_struct boot_loader_data_copy;

stivale2_struct_tag_framebuffer tag_framebuffer_copy;

stivale2_tag *stivale2_find_tag(uint64_t tag_id)
{
    stivale2_tag *current = (stivale2_tag *)boot_loader_data_copy.tags;
    while (current != NULL)
    {
        if (current->identifier == tag_id)
        {
            return current;
        }

        current = (stivale2_tag *)current->next;
    }
    return NULL;
}

void start_process();

/*
//  ____    __    ____  __  .__   __.   _______      ______        _______.
//  \   \  /  \  /   / |  | |  \ |  |  /  _____|    /  __  \      /       |
//   \   \/    \/   /  |  | |   \|  | |  |  __     |  |  |  |    |   (----`
//    \            /   |  | |  . `  | |  | |_ |    |  |  |  |     \   \
//     \    /\    /    |  | |  |\   | |  |__| |    |  `--'  | .----)   |
//      \__/  \__/     |__| |__| \__|  \______|     \______/  |_______/
*/

size_t get_cpu_count()
{
    return smp::the()->processor_count;
}

size_t get_current_cpu_id()
{
    return apic::the()->get_current_processor_id();
}

ASM_FUNCTION void kernel_start(struct stivale2_struct *bootloader_data)
{
    asm volatile("and rsp, -16");
    asm volatile("cli");
    // fs is used for getting cpu n°
    asm volatile("mov ax, 0");
    asm volatile("mov fs, ax");
    com_device com = com_device();
    com.init(COM_PORT::COM1);

    // get_current_cpu()->interrupt_stack = get_current_cpu()->stack_data_interrupt;

    init_sse();

    setup_gdt();
    gdt_descriptors[get_current_cpu_id()].debug_out();
    init_idt();
    if (has_xsave())
    {
        log("xsave", LOG_INFO, "cpu has support for xsave");
    }
    if (has_avx())
    {
        log("avx", LOG_INFO, "cpu has support for avx");
    }
    memcpy(&boot_loader_data_copy, bootloader_data, sizeof(struct stivale2_struct));
    bootdat = ((uint64_t)&boot_loader_data_copy);
    tag_framebuffer_copy = *(stivale2_struct_tag_framebuffer *)stivale2_find_tag(STIVALE2_STRUCT_TAG_FRAMEBUFFER_ID);

    gdt_descriptors[get_current_cpu_id()].debug_out();

    stivale2_struct_tag_memmap *memmap_tag = (stivale2_struct_tag_memmap *)stivale2_find_tag(STIVALE2_STRUCT_TAG_MEMMAP_ID);

    init_physical_memory(memmap_tag);
    tss_init((uintptr_t)stack + sizeof(char) * STACK_SIZE);
    outw(0x8A00, 0x8A00);
    outw(0x8A00, 0x08AE0);
    init_vmm(memmap_tag);
    pic_init();

    RTC::the()->init();

    acpi::the()->init();

    acpi::the()->getFACP();

    madt::the()->init();

    apic::the()->init();

    smp::the()->init();

    apic_timer *timer = new apic_timer();
    timer->init();

    for (int i = 0; i < 16; i++)
    {
        apic::the()->set_redirect_irq(0, i, 1);
    }

    mboot_module::the()->init(bootloader_data);

    init_multi_process(start_process);
    asm volatile("sti");
    while (true)
    {

        asm("hlt");
    }
}

void start_process()
{
    basic_framebuffer_graphic_device *framebuff = new basic_framebuffer_graphic_device(tag_framebuffer_copy.framebuffer_width, tag_framebuffer_copy.framebuffer_height, tag_framebuffer_copy.framebuffer_addr);

    add_device(framebuff);

    if (ata_driver::has_ata_device())
    {
        ata_driver *me = new ata_driver();
        me->init();
        add_io_device(me);
    }

    pci_system *pci = new pci_system();
    pci->init();

    //   launch_programm("initfs/memory_service.exe", main_fs_system::the()->main_fs());

    _start((stivale_struct *)bootdat);
}
