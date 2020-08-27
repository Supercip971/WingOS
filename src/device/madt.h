#pragma once
#include <device/acpi.h>

enum MADT_type
{
    MADT_LAPIC = 0,
    MADT_IOAPIC = 1,
    MADT_ISO = 2,
    MADT_NMI = 4,
    MADT_LAPIC_OVERRIDE = 5
};

struct MADT_record_table_entry
{
    uint8_t ttype; // MADT_type
    uint8_t tlenght;
} __attribute__((packed));

// entry type 0

struct MADT_table_LAPIC
{
    MADT_record_table_entry standart;
    uint8_t processor_id;
    uint8_t apic_id;
    uint32_t misc_flag;
} __attribute__((packed));

// entry type 1

struct MADT_table_IOAPIC
{
    MADT_record_table_entry standart;
    uint8_t IOAPIC_id;
    uint8_t reserved;
    uint32_t ioapic_addr;
    uint32_t interrupt_base;
} __attribute__((packed));

// entry type 2

struct MADT_table_ISO
{
    MADT_record_table_entry standart;
    uint8_t bus;
    uint8_t irq;
    uint32_t interrupt;
    uint32_t misc_flags;
} __attribute__((packed));

// entry type 4

struct MADT_table_NMI
{
    MADT_record_table_entry standart;
    uint8_t processor_id;
    uint16_t flags;
    uint8_t LINT;
} __attribute__((packed));

// entry type 5

struct MADT_table_LAPICO
{
    MADT_record_table_entry standart;
    uint16_t reserved;
    uint64_t apic_address;
} __attribute__((packed));

struct MADT_head
{
    RSDTHeader RShead;

    uint32_t lapic;
    uint32_t flags;
    MADT_record_table_entry MADT_table[];
} __attribute__((packed));

class madt
{
    void *madt_address = 0x0;
    MADT_head *madt_header = 0x0;

public:
    madt();
    void init();
    uint64_t get_madt_table_lenght();
    MADT_record_table_entry *get_madt_table_record();
    static madt *the();
};
