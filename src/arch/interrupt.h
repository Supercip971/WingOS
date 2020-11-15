#pragma once
#include <arch/64bit.h>
#include <arch/arch.h>
#include <stdint.h>

#define IDT_ENTRY_COUNT 256
#define INTGATE 0x8e
#define TRAPGATE 0xeF

typedef void (*irq_handler_func)(unsigned int irq);
struct idt_entry
{
    uint16_t offset_low16;
    uint16_t cs;
    uint8_t ist;
    uint8_t attributes;
    uint16_t offset_mid16;
    uint32_t offset_high32;
    uint32_t zero;
} __attribute__((packed));
struct idtr
{
    uint16_t size;   // size of the IDT
    uint64_t offset; // address of the IDT
} __attribute__((packed));

void init_idt(void);

void add_irq_handler(irq_handler_func func, unsigned int irq_target);
