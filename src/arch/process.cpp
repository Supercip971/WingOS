#include <arch/64bit.h>
#include <arch/gdt.h>
#include <arch/mem/liballoc.h>
#include <arch/process.h>
#include <com.h>
#pragma GCC optimize("-O0")
extern "C" void irq0_first_jump();
process *process_array;
process kernel_process;
process *current_process = nullptr;
bool process_locked = true;
bool process_loaded = false;
void lock_process() { process_locked = true; }
void unlock_process() { process_locked = false; }
void main_process_1()
{
    while (true)
    {
        asm("hlt");
    }
}
void main_process_2()
{
    while (true)
    {
        asm("hlt");
    }
}

void init_multi_process(func start)
{
    printf("loading process \n");
    process_array = (process *)malloc(sizeof(process) * MAX_PROCESS);
    printf("loading process 0 \n");
    for (int i = 0; i < MAX_PROCESS; i++)
    {
        process_array[i].pid = i;
        process_array[i].current_process_state = process_state::PROCESS_AVAILABLE;
        for (int j = 0; j < PROCESS_STACK_SIZE; j++)
        {
            process_array[i].stack[j] = 0;
        }
    }

    printf("loading process 1 \n");
    init_process(main_process_1);
    printf("loading process 3 \n");
    init_process(main_process_2);
    printf("loading process 2 \n");
    init_process(start);
    process_loaded = true;

    unlock_process();
    asm volatile("jmp irq0_first_jump");
}

process *init_process(func entry_point)
{
    for (int i = 0; i < MAX_PROCESS; i++)
    {
        if (process_array[i].current_process_state ==
            process_state::PROCESS_AVAILABLE)
        {
            process_array[i].current_process_state = process_state::PROCESS_WAITING;
            process_array[i].entry_point = (uint64_t)entry_point;
            process_array[i].rsp =
                ((uint64_t)process_array[i].stack) + PROCESS_STACK_SIZE;

            uint64_t *rsp = (uint64_t *)process_array[i].rsp;
            rsp--;
            uint64_t crsp = (uint64_t)rsp;
            *rsp-- = SLTR_KERNEL_DATA;      // SS
            *rsp-- = crsp;                  // RSP
            *rsp-- = 0x286;                 // RFLAGS
            *rsp-- = SLTR_KERNEL_CODE;      // CS
            *rsp-- = (uint64_t)entry_point; // RIP
            *rsp-- = 0;
            *rsp-- = 0;
            *rsp-- = 0;
            *rsp-- = 0;
            *rsp-- = 0;
            *rsp-- = 0;
            *rsp-- = 0;
            *rsp-- = 0;
            *rsp-- = 0;
            *rsp-- = 0;
            *rsp-- = 0;
            *rsp-- = 0;
            *rsp-- = 0;
            *rsp-- = 0;
            *rsp = 0;
            process_array[i].rsp = (uint64_t)rsp;
            if (current_process == 0x0)
            {
                current_process = &process_array[i];
            }
            return &process_array[i];
        }
    }
    printf("no free process found \n");
    return nullptr;
}

void switch_context(InterruptStackFrame *current_Isf, process *next)
{
    if (next == NULL)
    {
        return; // early return
    }
    if (current_process == NULL)
    {
        current_Isf = (InterruptStackFrame *)next->rsp;
        next->current_process_state = process_state::PROCESS_RUNNING;
        current_process = next;
    }
    else
    {
        current_process->current_process_state = PROCESS_WAITING;
        current_process->rsp = current_Isf->rsp;

        current_Isf = (InterruptStackFrame *)next->rsp;
        next->current_process_state = process_state::PROCESS_RUNNING;
        current_process = next;
    }
}

process *get_next_process(uint64_t current_id)
{
    if (process_locked)
    {
        printf("return process cauz' they are lock \n");
        return current_process;
    }
    for (uint64_t i = current_id; i < MAX_PROCESS; i++)
    {
        if (process_array[i].current_process_state ==
            process_state::PROCESS_AVAILABLE)
        {
            break;
        }
        else if (process_array[i].current_process_state ==
                 process_state::PROCESS_WAITING)
        {
            return &process_array[i];
        }
    }
    for (uint64_t i = 0; i < current_id; i++)
    { // we do a loop
        if (process_array[i].current_process_state ==
            process_state::PROCESS_WAITING)
        {
            return &process_array[i];
        }
    }
    printf("no process found \n");
    return nullptr;
}

void irq_0_process_handler(InterruptStackFrame *isf)
{
    if (process_locked)
    {
        return;
    }
    if (current_process == NULL)
    {
        process *i = get_next_process(0);
        if (i == 0)
        {
            return;
        }
        switch_context(isf, i);
    }
    else
    {
        process *i = get_next_process(current_process->pid);
        if (i == nullptr)
        {
            return;
        }
        switch_context(isf, i);
    }
}
char temp_esp[8192];
process *nxt;
extern "C" uint64_t get_current_esp()
{
    if (current_process == nullptr)
    {
        printf("oh ow do you want some cookie ? \n");
        return (uint64_t)temp_esp + PROCESS_STACK_SIZE;
    }
    else
    {
        return (uint64_t)current_process;
    }
}
extern "C" uint64_t get_next_esp()
{
    if (current_process == 0)
    {
        nxt = get_next_process(0);
        return (uint64_t)nxt;
    }
    else
    {
        nxt = get_next_process(current_process->pid);
        return (uint64_t)nxt;
    }
}
extern "C" void reload_cr3();
extern "C" void task_update_switch(process *next)
{
    tss_set_rsp0((uint64_t)nxt->stack + PROCESS_STACK_SIZE - 8);
    current_process->current_process_state = process_state::PROCESS_WAITING;
    current_process = nxt;
    next->current_process_state = process_state::PROCESS_RUNNING;
}
