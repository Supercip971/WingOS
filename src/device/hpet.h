#ifndef HPET_H
#define HPET_H
#include <device/acpi.h>
#include <stdint.h>

enum HPET_config_data
{
    H_ENABLE_INTERRUPT = (1 << 2),
    H_ENABLE_PERIODIC_MODE = (1 << 3),
    H_PERIODIC_MODE_SUPPORT = (1 << 4),
    H_SIZE = (1 << 5),
    H_SET_PERIODIC_ACCUMULATOR = (1 << 6),

};
// why use an enum only for one entry ?

#define GENERAL_LEGACY_REPLACEMENT (1 << 15)

// thank for QWORD and osdev :D
struct address_structure
{
    uint8_t address_space_id; // 0 - system memory, 1 - system I/O
    uint8_t register_bit_width;
    uint8_t register_bit_offset;
    uint8_t reserved;
    uint64_t address;
} __attribute__((packed));

struct hpet_individual_timer
{
    volatile uint64_t cfg_what_can_i_do;
    volatile uint64_t comp_value;
    volatile uint64_t int_route;
    volatile uint64_t i_dont_exist;
};

struct main_hpet_struct
{
    volatile uint64_t general_capabilities;
    volatile uint64_t i_dont_exist;
    volatile uint64_t general_configuration;
    volatile uint64_t i_dont_exist1;
    volatile uint64_t general_int_status;
    volatile uint64_t i_dont_exist2;
    volatile uint64_t i_dont_exist3[2][12];
    volatile uint64_t main_counter_value;
    volatile uint64_t i_dont_exist4;
    struct hpet_individual_timer timers[];
} __attribute__((packed));
struct entry_hpet
{

    RSDTHeader RShead;
    uint8_t hardware_rev_id;
    uint8_t comparator_count : 5;
    uint8_t counter_size : 1;
    uint8_t reserved : 1;
    uint8_t legacy_replacement : 1;
    uint16_t pci_vendor_id;
    address_structure address;
    uint8_t hpet_number;
    uint16_t minimum_tick;
    uint8_t page_protection;
} __attribute__((packed));

class hpet
{
    entry_hpet *main_hpet_entry;
    main_hpet_struct *hpet_main_structure;

public:
    hpet();

    void init_hpet();
    static hpet *the();
};

#endif // HPET_H
