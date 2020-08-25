#pragma once
#include <int_value.h>
#include <arch/64bit.h>
#define MAX_PROCESS 128
#define PROCESS_STACK_SIZE 4096
enum process_state{
    PROCESS_AVAILABLE = 0,
    PROCESS_RUNNING = 1,
    PROCESS_WAITING = 2,
    PROCESS_CRASH = 3
};
struct process{
    uint64_t rsp = 0;
    process_state current_process_state;
    uint64_t pid = 0;
    uint8_t stack[PROCESS_STACK_SIZE];
    uint64_t entry_point = 0x0;
}__attribute__((packed));
typedef void (*func) ();

process* init_process(func entry_point);

void init_multi_process();

void unlock_process();
void lock_process();

void irq_0_process_handler(InterruptStackFrame* isf);
