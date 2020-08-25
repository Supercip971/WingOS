#include <arch/gdt.h>
#include <com.h>
#include <kernel.h>
#include <stddef.h>
#pragma GCC optimize("-O0")
/* flags */
#define GDT_CS 0x18
#define GDT_DS 0x10
#define GDT_TSS 0x09
#define GDT_WRITABLE 0x02
#define GDT_USER 0x60
#define GDT_PRESENT 0x80

/* granularity */
#define GDT_LM 0x2
gdtr_t gdtr;

tss_t tss;

tss_t *get_tss() { return &tss; }

gdt_descriptor_t gdt_descriptors[64];
static void gdt_set_descriptor(gdt_descriptor_t *gdt_descriptors, uint16_t sel,
                               uint8_t flags, uint8_t gran)
{
    gdt_descriptor_t *descriptor =
        &gdt_descriptors[sel / sizeof(*gdt_descriptors)];
    descriptor->flags = flags;
    descriptor->granularity = (gran << 4) | 0x0F;
    descriptor->limit_low = 0xFFFF;
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
    com_write_str("rgdt_init");

    uint64_t tss_base = (uint64_t)&tss;
    uint64_t tss_limit = tss_base + sizeof(tss) - 1;

    com_write_str("reset gdt");

    memzero(&gdt_descriptors, sizeof(gdt_descriptors) * 64);

    com_write_str("set gdt entries");
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

    gdtr.addr = (uint64_t)&gdt_descriptors;
    gdtr.len = sizeof(gdt_descriptors) * GDT_DESCRIPTORS - 1;
    gdtr_install(&gdtr, SLTR_KERNEL_CODE, SLTR_KERNEL_DATA);
}

void tss_init(uint64_t i)
{
    memzero(&tss, sizeof(tss));
    tss.iomap_base = sizeof(tss) - 1;
    tss.rsp0 = (uint64_t)i;

    asm volatile("mov ax, %0 \n ltr ax"
                 :
                 : "i"(SLTR_TSS)
                 : "rax");
}

void tss_set_rsp0(uint64_t rsp0) { tss.rsp0 = rsp0; }
void setup_gdt(unsigned long i) { rgdt_init(); }
