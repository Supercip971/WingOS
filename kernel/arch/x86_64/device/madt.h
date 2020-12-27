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
    uint32_t gsib;
} __attribute__((packed));

// entry type 2

struct MADT_table_ISO
{
    MADT_record_table_entry standart;
    uint8_t bus;
    uint8_t irq;        // source
    uint32_t interrupt; // gsi
    uint16_t misc_flags;
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

public:
    uint64_t lapic_base = 0x0;
    madt();
    MADT_head *madt_header = 0x0;
    void log_all();
    void init();
    uint64_t get_madt_table_lenght();
    MADT_record_table_entry *get_madt_table_record();
    MADT_table_IOAPIC **get_madt_ioAPIC();
    MADT_table_ISO **get_madt_ISO();
    static madt *the();
};
