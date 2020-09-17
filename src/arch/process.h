#pragma once
#include <arch/64bit.h>
#include <int_value.h>
#define MAX_PROCESS 128
#define PROCESS_STACK_SIZE 8196
enum process_state
{
    PROCESS_AVAILABLE = 0,
    PROCESS_RUNNING = 1,
    PROCESS_WAITING = 2,
    PROCESS_CRASH = 3,
    PROCESS_NOT_STARTED = 4
};

#define MAX_PROCESS_MEMORY_DATA_MAP 64
struct process_memory_data_map
{
    uint64_t from;
    uint64_t to;
    uint64_t length;
    bool used = false;
};

struct process
{
    uint64_t rsp = 0;
    process_state current_process_state;
    uint64_t pid = 0;
    uint8_t stack[PROCESS_STACK_SIZE];
    uint64_t entry_point = 0x0;

    process_memory_data_map mmap[MAX_PROCESS_MEMORY_DATA_MAP];
} __attribute__((packed));
static process *current_process = nullptr;
// TODO: make a get_current_process() function
typedef void (*func)();
void add_thread_map(process *p, uint64_t from, uint64_t to, uint64_t length);
process *init_process(func entry_point, bool start_direct);

void init_multi_process(func start);

void unlock_process();
void lock_process();

extern "C" void irq_0_process_handler(InterruptStackFrame *isf);
