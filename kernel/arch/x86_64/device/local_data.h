#pragma once
#include <64bit.h>
#include <arch.h>
#include <backtrace.h>
#include <device/apic.h>
#include <gdt.h>
#include <interrupt.h>
#include <mem/virtual.h>
#include <proc/process.h>
#include <smp.h>
#include <stdint.h>
class page_table;
static const uint32_t stack_size = STACK_SIZE;
class cpu
{
public:
    uint8_t *syscall_stack;
    uint64_t saved_stack;
    uint64_t current_processor_id;

    idtr cidt;

    gdtr cgdt;

    tss ctss;

    uint8_t stack_data[stack_size] PAGE_ALIGN;
    uint8_t *stack_data_interrupt;

    uint64_t lapic_id;
    page_table *cpu_page_table;

    uint64_t fpu_data[128] __attribute__((aligned(16)));

    void load_sse(uint64_t *data);
    void save_sse(uint64_t *data);

    backtrace local_backtrace;
} __attribute__((packed));
//local_data *get_current_data();
//local_data *get_current_data(int id);
extern cpu procData[smp::max_cpu];

inline cpu *get_current_cpu()
{
    uint64_t cc = 0;
    asm volatile("mov %0, fs \n"
                 : "=r"(cc));

    return &procData[cc];
}

inline cpu *get_current_cpu(int id)
{

    return &procData[id];
}
