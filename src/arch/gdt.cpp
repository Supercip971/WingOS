#include <arch/gdt.h>

#include <arch/mem/memory_manager.h>
#include <com.h>
#include <device/local_data.h>
#include <kernel.h>
#include <logging.h>
#pragma GCC optimize("-O0")
/* flags */
tss_t tss;

gdt_descriptor_t gdt_descriptors[64];

tss_t *get_tss() { return &tss; };

char tss_ist1[8192] __attribute__((aligned(16)));
char tss_ist2[8192] __attribute__((aligned(16)));
char tss_ist3[8192] __attribute__((aligned(16)));
void tss_set_rsp0(uint64_t rsp0)
{
    tss.rsp0 = rsp0;
};
void rgdt_init(void);
void setup_gdt(unsigned long i) { rgdt_init(); }

static void gdt_set_descriptor(gdt_descriptor_t *gdt_descriptors, uint16_t sel,
                               uint8_t flags, uint8_t gran)
{
    gdt_descriptor_t *descriptor =
        &gdt_descriptors[sel / sizeof(*gdt_descriptors)];
    descriptor->flags = flags;
    descriptor->granularity = (gran << 4) | 0x0F;
    descriptor->limit_low = 0xFFFF;

    log("gdt", LOG_INFO) << "setup gdt descriptor = " << *((uint64_t *)descriptor);
}

static void gdt_set_xdescriptor(gdt_descriptor_t *gdt_descriptors, uint16_t sel,
                                uint8_t flags, uint8_t gran, uint64_t base,
                                uint64_t limit)
{
    gdt_xdescriptor_t *descriptor =
        (gdt_xdescriptor_t *)(&gdt_descriptors[sel / sizeof(*gdt_descriptors)]);

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

extern "C" void gdtr_install(gdtr_t *, unsigned short, unsigned short);
void __attribute__((optimize("O0"))) rgdt_init(void)
{
    log("gdt", LOG_DEBUG) << "loading gdt";

    uint64_t tss_base = (uint64_t)&get_current_cpu()->tss;
    uint64_t tss_limit = tss_base + sizeof(get_current_cpu()->tss) - 1;

    log("gdt", LOG_INFO) << "resetting gdt";

    memzero(&gdt_descriptors, sizeof(gdt_descriptors) * 64);

    log("gdt", LOG_INFO) << "setting gdt entry";
    gdt_set_descriptor(gdt_descriptors, SLTR_KERNEL_CODE, GDT_PRESENT | GDT_CS,
                       GDT_LM);
    gdt_set_descriptor(gdt_descriptors, SLTR_KERNEL_DATA,
                       GDT_PRESENT | GDT_DS | GDT_WRITABLE, 0);
    gdt_set_descriptor(gdt_descriptors, SLTR_USER_DATA,
                       GDT_PRESENT | GDT_DS | GDT_USER | GDT_WRITABLE, 0);
    gdt_set_descriptor(gdt_descriptors, SLTR_USER_CODE,
                       GDT_PRESENT | GDT_CS | GDT_USER, GDT_LM);
    gdt_set_xdescriptor(gdt_descriptors, SLTR_TSS, GDT_PRESENT | GDT_TSS, 0,
                        tss_base, tss_limit);

    get_current_cpu()->gdt.addr = (uint64_t)&gdt_descriptors;
    get_current_cpu()->gdt.len = sizeof(gdt_descriptors) * GDT_DESCRIPTORS - 1;

    gdtr_install(&get_current_cpu()->gdt, SLTR_KERNEL_CODE, SLTR_KERNEL_DATA);
}

void tss_init(uint64_t i)
{
    memzero(&get_current_cpu()->tss, sizeof(tss));
    get_current_cpu()->tss.iomap_base = sizeof(tss);
    get_current_cpu()->tss.rsp0 = (uint64_t)i;
    get_current_cpu()->tss.ist1 = (uint64_t)tss_ist1 + 8192;
    get_current_cpu()->tss.ist2 = (uint64_t)tss_ist2 + 8192;
    get_current_cpu()->tss.ist3 = (uint64_t)tss_ist3 + 8192;

    asm volatile("mov ax, %0 \n ltr ax"
                 :
                 : "i"(SLTR_TSS)
                 : "rax");
}

void gdt_ap_init()
{

    log("gdt ap", LOG_DEBUG) << "loading gdt for ap";

    uint64_t tss_base = (uint64_t)&get_current_cpu()->tss;
    uint64_t tss_limit = tss_base + sizeof(get_current_cpu()->tss);

    log("gdt ap", LOG_INFO) << "resetting gdt";

    gdt_descriptor_t *new_gdt_descriptors = get_current_cpu()->gdt_descriptors;

    log("gdt ap", LOG_INFO) << "resetting gdt 2";
    memzero(new_gdt_descriptors, sizeof(gdt_descriptor_t) * 32);
    gdtr_t *d = &get_current_cpu()->gdt;
    memzero(d, sizeof(gdtr_t));

    log("gdt ap", LOG_INFO) << "setting gdt entry";
    gdt_set_descriptor(new_gdt_descriptors, SLTR_KERNEL_CODE, GDT_PRESENT | GDT_CS,
                       GDT_LM);
    gdt_set_descriptor(new_gdt_descriptors, SLTR_KERNEL_DATA,
                       GDT_PRESENT | GDT_DS | GDT_WRITABLE, 0);
    gdt_set_descriptor(new_gdt_descriptors, SLTR_USER_DATA,
                       GDT_PRESENT | GDT_DS | GDT_USER | GDT_WRITABLE, 0);
    gdt_set_descriptor(new_gdt_descriptors, SLTR_USER_CODE,
                       GDT_PRESENT | GDT_CS | GDT_USER, GDT_LM);
    gdt_set_xdescriptor(new_gdt_descriptors, SLTR_TSS, GDT_PRESENT | GDT_TSS, 0,
                        tss_base, tss_limit);

    get_current_cpu()->gdt.addr = (uint64_t)new_gdt_descriptors;
    get_current_cpu()->gdt.len = sizeof(gdt_descriptor_t) * GDT_DESCRIPTORS - 1;

    gdtr_install(&get_current_cpu()->gdt, SLTR_KERNEL_CODE, SLTR_KERNEL_DATA);
    memzero(&get_current_cpu()->tss, sizeof(tss));
    get_current_cpu()->tss.iomap_base = sizeof(tss);
    get_current_cpu()->tss.rsp0 = (uint64_t)POKE(0x570);
    get_current_cpu()->tss.ist1 = (uint64_t)get_current_cpu()->stack_data_interrupt + 8192;

    asm volatile("mov ax, %0 \n ltr ax"
                 :
                 : "i"(SLTR_TSS)
                 : "rax");
}
