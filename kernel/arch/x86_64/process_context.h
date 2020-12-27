#pragma once
#include <64bit.h>
#include <arch.h>
#include <process.h>
#include <stdint.h>

struct process;
void send_switch_process_to_all_cpu();

uintptr_t switch_context(InterruptStackFrame *current_Isf, process *next);
void add_thread_map(process *p, uintptr_t from, uintptr_t to, uint64_t length);

inline void yield()
{
    asm volatile("int 100");
}

void init_process_stackframe(process *pro, func entry_point);

uint64_t interpret_cpu_request(uint64_t cpu); // with architecture with no multi cpu return always 0

void init_process_paging(process *pro, bool is_user);

void init_process_arch_ext(process *pro);
