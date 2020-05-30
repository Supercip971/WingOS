#include <stdint.h>
struct gdt_pointer {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

struct gdt_descriptor {
    uint16_t limit_low16;  // The low 16 bits of the limit of the segment
    uint16_t base_low16;   // The low 16 bits of the base of the segment
    uint8_t  base_mid8;    // The middle 8 bits of the base of the segment
    uint8_t access_byte;        // Flags, more info below
    uint8_t Granularity;        // Flags, more info below
    uint8_t  base_high8;   // the high 8 bits of the base of the segment
} __attribute__((packed));

struct tss {
    uint32_t reserved0; uint64_t rsp0;      uint64_t rsp1;
    uint64_t rsp2;      uint64_t reserved1; uint64_t ist1;
    uint64_t ist2;      uint64_t ist3;      uint64_t ist4;
    uint64_t ist5;      uint64_t ist6;      uint64_t ist7;
    uint64_t reserved2; uint16_t reserved3; uint16_t iopb_offset;
} ;
void setup_gdt();