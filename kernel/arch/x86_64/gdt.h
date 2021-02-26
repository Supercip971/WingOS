#pragma once
#include <stdint.h>
#include <stivale_struct.h>
#include <string.h>
#define GDT_DESCRIPTORS 7
enum gdt_selector : uint16_t
{
    NULL_SELECTOR = 0,
    KERNEL_CODE = 0x8,
    KERNEL_DATA = 0x10,
    USER_DATA = 0x1b,
    USER_CODE = 0x23,
    TSS_SELECTOR = 0x28,
};
enum gdt_flags : uint8_t
{
    WRITABLE = 0b10,
    USER = 0b1100000,
    PRESENT = 0b10000000,
    TSS = 0b1001,
    DS = 0b10000,
    CS = 0b11000,
};

enum gdt_granularity : uint8_t
{
    LONG_MODE_GRANULARITY = 0x2,
};

struct gdtr
{
    uint16_t len;
    uint64_t addr;
} __attribute__((packed));

struct gdt_descriptor
{
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_mid;
    uint8_t flags;
    uint8_t granularity; /* and high limit */
    uint8_t base_high;

    gdt_descriptor(){};
    gdt_descriptor(uint8_t flag, uint8_t gran)
    {
        base_high = 0;
        base_mid = 0;
        base_low = 0;
        flags = flag | gdt_flags::PRESENT;
        granularity = (gran << 4) | 0x0F;
        limit_low = 0;
    }
} __attribute__((packed));

struct gdt_xdescriptor
{
    gdt_descriptor low;

    struct
    {
        uint32_t base_xhigh;
        uint32_t reserved;
    } high;

    gdt_xdescriptor(uint8_t flag, uintptr_t base, uintptr_t limit)
    {
        low.flags = flag | gdt_flags::PRESENT;
        low.granularity = (0 << 4) | ((limit >> 16) & 0x0F);
        low.limit_low = limit & 0xFFFF;
        low.base_low = base & 0xFFFF;
        low.base_mid = ((base >> 16) & 0xFF);
        low.base_high = ((base >> 24) & 0xFF);
        high.base_xhigh = ((base >> 32) & 0xFFFFFFFF);
        high.reserved = 0;
    }
} __attribute__((packed));

struct tss
{
    uint32_t reserved0;
    uint64_t rsp0;
    uint64_t rsp1;
    uint64_t rsp2;
    uint64_t reserved1;
    uint64_t ist1;
    uint64_t ist2;
    uint64_t ist3;
    uint64_t ist4;
    uint64_t ist5;
    uint64_t ist6;
    uint64_t ist7;
    uint64_t reserved2;
    uint16_t reserved3;
    uint16_t iomap_base;
} __attribute__((packed));

#define GDT_ARRAY_SEL(a) a / sizeof(gdt_descriptor)

template <int entry_count>
class gdt_descriptor_array
{
    gdt_descriptor entrys[entry_count];

public:
    constexpr size_t size() const
    {
        return sizeof(gdt_descriptor) * entry_count;
    }
    void zero()
    {
        memset(entrys, 0, sizeof(entrys[0]) * entry_count);
    }
    gdt_descriptor *get_entry(gdt_selector entry)
    {
        return &entrys[GDT_ARRAY_SEL(entry)];
    }
    gdt_xdescriptor *get_entry_x(gdt_selector entry)
    {
        return (gdt_xdescriptor *)&entrys[GDT_ARRAY_SEL(entry)];
    }
    void set(gdt_selector entry, gdt_descriptor desc)
    {
        *get_entry(entry) = desc;
    }
    void xset(gdt_selector entry, gdt_xdescriptor desc)
    {
        *get_entry_x(entry) = desc;
    }
    gdt_descriptor *raw()
    {
        return entrys;
    }
    void fill_gdt_register(gdtr *target)
    {
        target->len = size();
        target->addr = (uintptr_t)raw();
    }
};

void tss_init(uint64_t i);
void tss_set_rsp0(uint64_t rsp0);

void gdt_init();
void setup_gdt();
void gdt_ap_init();
