#include <arch/gdt.h>

#include <arch/mem/memory_manager.h>
#include <com.h>
#include <device/local_data.h>
#include <kernel.h>
#include <logging.h>
gdt_descriptor gdt_descriptors[64];

char tss_ist1[8192] __attribute__((aligned(16)));
char tss_ist2[8192] __attribute__((aligned(16)));
char tss_ist3[8192] __attribute__((aligned(16)));

ASM_FUNCTION void gdtr_install(gdtr *, unsigned short, unsigned short);

void tss_set_rsp0(uint64_t rsp0)
{
    get_current_cpu()->ctss.rsp0 = rsp0;
};

static void gdt_set_descriptor(gdt_descriptor *gdt_descriptors, uint16_t sel,
                               uint8_t flags, uint8_t gran)
{
    gdt_descriptor *descriptor =
        &gdt_descriptors[sel / sizeof(*gdt_descriptors)];
    descriptor->flags = flags;
    descriptor->granularity = (gran << 4) | 0x0F;
    descriptor->limit_low = 0xFFFF;

    log("gdt", LOG_INFO) << "setup gdt descriptor = " << *((uint64_t *)descriptor);
}

static void gdt_set_xdescriptor(gdt_descriptor *gdt_descriptors, uint16_t sel,
                                uint8_t flags, uint8_t gran, uintptr_t base,
                                uintptr_t limit)
{
    gdt_xdescriptor *descriptor =
        (gdt_xdescriptor *)(&gdt_descriptors[sel / sizeof(*gdt_descriptors)]);

    descriptor->low.flags = flags;
    descriptor->low.granularity = (gran << 4) | ((limit >> 16) & 0x0F);
    descriptor->low.limit_low = limit & 0xFFFF;
    descriptor->low.base_low = base & 0xFFFF;
    descriptor->low.base_mid = ((base >> 16) & 0xFF);
    descriptor->low.base_high = ((base >> 24) & 0xFF);
    descriptor->high.base_xhigh = ((base >> 32) & 0xFFFFFFFF);
    descriptor->high.reserved = 0;
    log("gdt", LOG_INFO) << "setup xgdt descriptor = " << *((uint64_t *)descriptor);
}

void __attribute__((optimize("O0"))) setup_gdt()
{
    log("gdt", LOG_DEBUG) << "loading gdt";

    uintptr_t tss_base = (uintptr_t)&get_current_cpu()->ctss;
    uintptr_t tss_limit = tss_base + sizeof(get_current_cpu()->ctss);

    log("gdt", LOG_INFO) << "resetting gdt";

    memzero(&gdt_descriptors, sizeof(gdt_descriptors[0]) * 64);

    log("gdt", LOG_INFO) << "setting gdt entry";
    gdt_set_descriptor(gdt_descriptors, gdt_selector::KERNEL_CODE, gdt_flags::PRESENT | gdt_flags::CS,
                       gdt_granularity::LONG_MODE_GRANULARITY);
    gdt_set_descriptor(gdt_descriptors, gdt_selector::KERNEL_DATA,
                       gdt_flags::PRESENT | gdt_flags::DS | gdt_flags::WRITABLE, 0);
    gdt_set_descriptor(gdt_descriptors, gdt_selector::USER_DATA,
                       gdt_flags::PRESENT | gdt_flags::DS | gdt_flags::USER | gdt_flags::WRITABLE, 0);
    gdt_set_descriptor(gdt_descriptors, gdt_selector::USER_CODE,
                       gdt_flags::PRESENT | gdt_flags::CS | gdt_flags::USER, gdt_granularity::LONG_MODE_GRANULARITY);
    gdt_set_xdescriptor(gdt_descriptors, gdt_selector::TSS_SELECTOR, gdt_flags::PRESENT | gdt_flags::TSS, 0,
                        tss_base, tss_limit);

    get_current_cpu()->cgdt.addr = (uint64_t)&gdt_descriptors;
    get_current_cpu()->cgdt.len = sizeof(gdt_descriptors[0]) * GDT_DESCRIPTORS;

    gdtr_install((&get_current_cpu()->cgdt), gdt_selector::KERNEL_CODE, gdt_selector::KERNEL_DATA);
}

void tss_init(uint64_t i)
{
    memzero(&get_current_cpu()->ctss, sizeof(tss));
    get_current_cpu()->ctss.iomap_base = sizeof(tss);
    get_current_cpu()->ctss.rsp0 = (uintptr_t)i;
    get_current_cpu()->ctss.ist1 = (uintptr_t)tss_ist1 + 8192;
    get_current_cpu()->ctss.ist2 = (uintptr_t)tss_ist2 + 8192;
    get_current_cpu()->ctss.ist3 = (uintptr_t)tss_ist3 + 8192;

    asm volatile("mov ax, %0 \n ltr ax"
                 :
                 : "i"(gdt_selector::TSS_SELECTOR)
                 : "rax");
}

void gdt_ap_init()
{

    log("gdt ap", LOG_DEBUG) << "loading gdt for ap";

    uintptr_t tss_base = (uintptr_t)&get_current_cpu()->ctss;
    uintptr_t tss_limit = tss_base + sizeof(get_current_cpu()->ctss);

    log("gdt ap", LOG_INFO) << "resetting gdt";

    gdt_descriptor *new_gdt_descriptors = get_current_cpu()->gdt_descriptors;

    memzero(new_gdt_descriptors, sizeof(gdt_descriptor) * 32);
    gdtr *d = &get_current_cpu()->cgdt;
    memzero(d, sizeof(gdtr));

    log("gdt ap", LOG_INFO) << "setting gdt entry";
    gdt_set_descriptor(new_gdt_descriptors, gdt_selector::KERNEL_CODE, gdt_flags::PRESENT | gdt_flags::CS,
                       gdt_granularity::LONG_MODE_GRANULARITY);
    gdt_set_descriptor(new_gdt_descriptors, gdt_selector::KERNEL_DATA,
                       gdt_flags::PRESENT | gdt_flags::DS | gdt_flags::WRITABLE, 0);
    gdt_set_descriptor(new_gdt_descriptors, gdt_selector::USER_DATA,
                       gdt_flags::PRESENT | gdt_flags::DS | gdt_flags::USER | gdt_flags::WRITABLE, 0);
    gdt_set_descriptor(new_gdt_descriptors, gdt_selector::USER_CODE,
                       gdt_flags::PRESENT | gdt_flags::CS | gdt_flags::USER, gdt_granularity::LONG_MODE_GRANULARITY);
    gdt_set_xdescriptor(new_gdt_descriptors, gdt_selector::TSS_SELECTOR, gdt_flags::PRESENT | gdt_flags::TSS, 0,
                        tss_base, tss_limit);

    get_current_cpu()->cgdt.addr = (uint64_t)new_gdt_descriptors;
    get_current_cpu()->cgdt.len = sizeof(gdt_descriptor) * GDT_DESCRIPTORS - 1;

    gdtr_install((&get_current_cpu()->cgdt), gdt_selector::KERNEL_CODE, gdt_selector::KERNEL_DATA);
    memzero(&get_current_cpu()->ctss, sizeof(tss));

    get_current_cpu()->ctss.iomap_base = sizeof(tss);
    get_current_cpu()->ctss.rsp0 = (uintptr_t)POKE(0x570);
    get_current_cpu()->ctss.ist1 = (uintptr_t)get_current_cpu()->stack_data_interrupt + 8192;

    asm volatile("mov ax, %0 \n ltr ax"
                 :
                 : "i"(gdt_selector::TSS_SELECTOR)
                 : "rax");
}
