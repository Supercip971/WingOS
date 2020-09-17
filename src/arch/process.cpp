#include <arch/64bit.h>
#include <arch/gdt.h>
#include <arch/mem/liballoc.h>
#include <arch/process.h>
#include <com.h>
#include <loggging.h>
#pragma GCC optimize("-O0")
extern "C" void irq0_first_jump();
extern "C" void reload_cr3();
process *process_array;
process kernel_process;
char temp_esp[8192];
process *nxt;
bool process_locked = true;
bool process_loaded = false;
void lock_process() { process_locked = true; }
void unlock_process() { process_locked = false; }
void main_process_1()
{
    printf("process 1 \n");
    asm("sti");
    while (true)
    {
        asm("int 32");
    }
}

void main_process_2()
{
    printf("process 2 \n");
    asm("sti");
    while (true)
    {

        asm("int 32");
    }
}
extern "C" void irq0_first_jump(void);
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
    init_process(main_process_1, true);

    printf("loading process 2 \n");
    init_process(main_process_2, true);

    printf("loading process 3 \n");
    init_process(start, true);
    process_loaded = true;

    asm volatile("sti");
    unlock_process();
    irq0_first_jump();
    //asm volatile("jmp irq0_first_jump");
}

process *init_process(func entry_point, bool start_direct)
{
    for (int i = 0; i < MAX_PROCESS; i++)
    {
        if (process_array[i].current_process_state ==
            process_state::PROCESS_AVAILABLE)
        {
            log("proc", LOG_INFO) << "adding process" << i << "entry : " << (uint64_t)entry_point;
            if (start_direct == true)
            {
                process_array[i].current_process_state = process_state::PROCESS_WAITING;
            }
            else
            {
                process_array[i].current_process_state = process_state::PROCESS_NOT_STARTED;
            }
            process_array[i].entry_point = (uint64_t)entry_point;
            process_array[i].rsp =
                ((uint64_t)process_array[i].stack) + PROCESS_STACK_SIZE;

            uint64_t *rsp = (uint64_t *)process_array[i].rsp;

            *rsp-- = 0;
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
            for (int j = 0; j < MAX_PROCESS_MEMORY_DATA_MAP; j++)
            {
                process_array[i].mmap[j].used = false;
            }
            return &process_array[i];
        }
    }
    printf("no free process found \n");
    return nullptr;
}

extern "C" void switch_context(InterruptStackFrame *current_Isf, process *next)
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

extern "C" process *get_next_process(uint64_t current_id)
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
                     process_state::PROCESS_WAITING &&
                 i != 0)
        {
            return &process_array[i];
        }
    }
    for (uint64_t i = 0; i < current_id; i++)
    { // we do a loop
        if (process_array[i].current_process_state ==
                process_state::PROCESS_WAITING &&
            i != 0)
        {
            return &process_array[i];
        }
    }
    printf("no process found \n");
    return nullptr;
}

extern "C" void irq_0_process_handler(InterruptStackFrame *isf)
{
    log("process", LOG_INFO) << "switching ";
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

extern "C" void task_update_switch(process *next)
{
    tss_set_rsp0((uint64_t)nxt->stack + PROCESS_STACK_SIZE - 8);
    current_process->current_process_state = process_state::PROCESS_WAITING;
    current_process = nxt;
    next->current_process_state = process_state::PROCESS_RUNNING;
    //   log("process", LOG_INFO) << "process entry : " << nxt->pid;

    for (int j = 0; j < MAX_PROCESS_MEMORY_DATA_MAP; j++)
    {
        if (nxt->mmap[j].used == true)
        {
            uint64_t mem_length = nxt->mmap[j].length;
            for (uint64_t i = 0; i < mem_length; i++)
            {
                log("process", LOG_INFO) << "mmap : " << nxt->mmap[j].from + i * PAGE_SIZE << " to : " << nxt->mmap[j].to + i * PAGE_SIZE;
                virt_map(nxt->mmap[j].from + i * PAGE_SIZE, nxt->mmap[j].to + i * PAGE_SIZE, 0x1 | 0x2 | 0x4);
            }
        }
    }

    update_paging();
}

void add_thread_map(process *p, uint64_t from, uint64_t to, uint64_t length)
{

    for (int j = 0; j < MAX_PROCESS_MEMORY_DATA_MAP; j++)
    {
        if (p->mmap[j].used == false)
        {
            p->mmap[j].used = true;

            p->mmap[j].from = from;
            p->mmap[j].to = to;
            p->mmap[j].length = length;
            return;
        }
    }
    log("process", LOG_ERROR) << "cant find a free map for thread";
}
