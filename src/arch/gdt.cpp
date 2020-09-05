#include <arch/gdt.h>
#include <com.h>
#include <device/local_data.h>
#include <kernel.h>
#include <stddef.h>
#pragma GCC optimize("-O0")
/* flags */
tss_t tss;

gdt_descriptor_t gdt_descriptors[64];

tss_t *get_tss() { return &tss; }
void tss_set_rsp0(uint64_t rsp0) { get_current_data()->tss.rsp0 = rsp0; }
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
    com_write_reg("setup gdt descriptor = ", *((uint64_t *)descriptor));
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
}

extern "C" void gdtr_install(gdtr_t *, unsigned short, unsigned short);
void __attribute__((optimize("O0"))) rgdt_init(void)
{
    printf("rgdt_init \n");

    uint64_t tss_base = (uint64_t)&get_current_data()->tss;
    uint64_t tss_limit = tss_base + sizeof(get_current_data()->tss) - 1;

    printf("reset gdt \n");

    memzero(&gdt_descriptors, sizeof(gdt_descriptors) * 64);

    printf("set gdt entries \n");
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

    get_current_data()->gdt.addr = (uint64_t)&gdt_descriptors;
    get_current_data()->gdt.len = sizeof(gdt_descriptors) * GDT_DESCRIPTORS - 1;

    gdtr_install(&get_current_data()->gdt, SLTR_KERNEL_CODE, SLTR_KERNEL_DATA);
}

void tss_init(uint64_t i)
{
    memzero(&get_current_data()->tss, sizeof(tss));
    get_current_data()->tss.iomap_base = sizeof(tss) - 1;
    get_current_data()->tss.rsp0 = (uint64_t)i;

    asm volatile("mov ax, %0 \n ltr ax"
                 :
                 : "i"(SLTR_TSS)
                 : "rax");
}

