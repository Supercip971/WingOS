#include <arch/64bit.h>
#include <arch/gdt.h>
#include <arch/lock.h>
#include <arch/mem/liballoc.h>
#include <arch/process.h>
#include <arch/smp.h>
#include <com.h>
#include <device/apic.h>
#include <device/local_data.h>
#include <kernel.h>
#include <loggging.h>
#include <utility.h>
#pragma GCC optimize("-O0")
extern "C" void irq0_first_jump();
extern "C" void reload_cr3();
process kernel_process;
char temp_esp[8192];
process *nxt;
bool process_locked = true;
bool process_loaded = false;
void lock_process() { process_locked = true; }
void unlock_process() { process_locked = false; }
lock_type task_lock = {0};
extern "C" void start_task()
{
    lock((&task_lock));
}
extern "C" void end_task()
{
    unlock((&task_lock));
}
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
    log("proc", LOG_DEBUG) << "loading multi processing";
    printf("loading process \n");
    process_array = (process *)malloc(sizeof(process) * MAX_PROCESS);
    get_current_data()->current_process = nullptr;
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

    log("proc", LOG_INFO) << "loading process 0";
    init_process(main_process_1, true, "testing1");

    log("proc", LOG_INFO) << "loading process 1";
    init_process(main_process_2, true, "testing2");

    log("proc", LOG_INFO) << "loading process 2";
    init_process(start, true, "kernel process");
    process_loaded = true;

    asm volatile("sti");
    unlock_process();
    irq0_first_jump();
    //asm volatile("jmp irq0_first_jump");
}

process *init_process(func entry_point, bool start_direct, const char *name)
{

    for (int i = 0; i < MAX_PROCESS; i++)
    {
        if (process_array[i].current_process_state ==
            process_state::PROCESS_AVAILABLE)
        {
            log("proc", LOG_INFO) << "adding process" << i << "entry : " << (uint64_t)entry_point << "name : " << name;
            if (start_direct == true)
            {
                process_array[i].current_process_state = process_state::PROCESS_WAITING;
            }
            else
            {
                process_array[i].current_process_state = process_state::PROCESS_NOT_STARTED;
            }
            int iname = 0;
            for (iname = 0; iname < 128 && name[iname] != 0; iname++)
            {
                process_array[i].process_name[iname] = name[iname];
            }
            process_array[i].last_message_used = 0;
            process_array[i].process_name[iname] = 0;
            process_array[i].entry_point = (uint64_t)entry_point;
            process_array[i].rsp =
                ((uint64_t)process_array[i].stack) + PROCESS_STACK_SIZE;
            for (uint64_t j = 0; j < MAX_PROCESS_MESSAGE_QUEUE; j++)
            {
                process_array[i].msg_list[j].entry_free_to_use = true;
                process_array[i].msg_list[j].message_id = j;
            }
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
            process_array[i].is_ORS = false;
            if (get_current_data()->current_process == 0x0)
            {
                get_current_data()->current_process = &process_array[i];
            }
            for (int j = 0; j < MAX_PROCESS_MEMORY_DATA_MAP; j++)
            {
                process_array[i].mmap[j].used = false;
            }
            return &process_array[i];
        }
    }
    log("proc", LOG_ERROR) << "no free process found";
    return nullptr;
}

extern "C" void switch_context(InterruptStackFrame *current_Isf, process *next)
{
    if (next == NULL)
    {
        return; // early return
    }
    if (get_current_data()->current_process == NULL)
    {
        current_Isf = (InterruptStackFrame *)next->rsp;
        next->current_process_state = process_state::PROCESS_RUNNING;
        get_current_data()->current_process = next;
    }
    else
    {
        get_current_data()->current_process->current_process_state = PROCESS_WAITING;
        get_current_data()->current_process->rsp = current_Isf->rsp;

        current_Isf = (InterruptStackFrame *)next->rsp;
        next->current_process_state = process_state::PROCESS_RUNNING;
        get_current_data()->current_process = next;
    }
}

extern "C" process *get_next_process(uint64_t current_id)
{
    if (process_locked)
    {
        return get_current_data()->current_process;
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

            if (process_array[i].is_ORS == true && process_array[i].should_be_active == false)
            {
                continue;
            }
            return &process_array[i];
        }
    }
    for (uint64_t i = 0; i < current_id; i++)
    { // we do a loop
        if (process_array[i].current_process_state ==
                process_state::PROCESS_WAITING &&
            i != 0)
        {
            if (process_array[i].is_ORS == true && process_array[i].should_be_active == false)
            {
                continue;
            }
            return &process_array[i];
        }
    }
    log("proc", LOG_ERROR) << "no free process found";
    return nullptr;
}

extern "C" void irq_0_process_handler(InterruptStackFrame *isf)
{
    if (process_locked)
    {
        return;
    }
    if (get_current_data()->current_process == NULL)
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
        process *i = get_next_process(get_current_data()->current_process->pid);
        if (i == nullptr)
        {
            return;
        }
        switch_context(isf, i);
    }
}

extern "C" uint64_t get_current_esp()
{
    if (get_current_data()->current_process == nullptr)
    {

        log("proc", LOG_ERROR) << "i can't describe this error file : " << __FILE__;
        //  printf("oh ow do you want some cookie ? \n");
        return (uint64_t)temp_esp + PROCESS_STACK_SIZE;
    }
    else
    {
        return (uint64_t)get_current_data()->current_process;
    }
}

extern "C" uint64_t get_next_esp()
{
    if (get_current_data()->current_process == 0)
    {
        nxt = get_next_process(0);
        return (uint64_t)nxt;
    }
    else
    {
        nxt = get_next_process(get_current_data()->current_process->pid);
        return (uint64_t)nxt;
    }
}


extern "C" void task_update_switch(process *next)
{
    //
    //move_to_next_cpu();
    // log("process", LOG_INFO) << "next : " << next->process_name;

    tss_set_rsp0((uint64_t)nxt->stack + PROCESS_STACK_SIZE - 8);
    get_current_data()->current_process->current_process_state = process_state::PROCESS_WAITING;
    get_current_data()->current_process = nxt;
    next->current_process_state = process_state::PROCESS_RUNNING;
    //   log("process", LOG_INFO) << "process entry : " << nxt->pid;

    for (int j = 0; j < MAX_PROCESS_MEMORY_DATA_MAP; j++)
    {
        if (nxt->mmap[j].used == true)
        {
            uint64_t mem_length = nxt->mmap[j].length;
            //      log("process", LOG_INFO) << " for next : " << next->process_name;

            for (uint64_t i = 0; i < mem_length; i++)
            {
                //        log("process", LOG_INFO) << "mmap : " << nxt->mmap[j].from + i * PAGE_SIZE << " to : " << nxt->mmap[j].to + i * PAGE_SIZE << "for proc " << nxt->process_name;
                map_page(nxt->mmap[j].from + i * PAGE_SIZE, nxt->mmap[j].to + i * PAGE_SIZE, 0x1 | 0x2 | 0x4);
            }
            update_paging();
        }
    }
    //  set_paging_dir(kernel_pagemap->pml4);
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

process_message *send_message(uint64_t data_addr, uint64_t data_length, const char *to_process)
{

    //log("process", LOG_INFO) << "sending message to " << to_process << "addr : " << data_addr << "length " << data_length;
    for (int i = 0; i < MAX_PROCESS; i++)
    {
        if (process_array[i].current_process_state == PROCESS_WAITING || process_array[i].current_process_state == PROCESS_RUNNING)
        {
            //    log("process", LOG_INFO) << "checking with " << process_array[i].process_name;

            if (strcmp(to_process, process_array[i].process_name) == 0)
            {
                for (int j = process_array[i].last_message_used; j < MAX_PROCESS_MESSAGE_QUEUE; j++)
                {
                    if (process_array[i].msg_list[j].entry_free_to_use == true)
                    {
                        process_array[i].last_message_used = j;
                        process_message *todo = &process_array[i].msg_list[j];
                        todo->entry_free_to_use = false;
                        todo->has_been_readed = false;
                        todo->content_length = data_length;
                        uint64_t memory_kernel_copy = (uint64_t)malloc(data_length);
                        memcpy((void *)memory_kernel_copy, (void *)data_addr, data_length);
                        todo->content_address = memory_kernel_copy;
                        todo->from_pid = get_current_data()->current_process->pid;
                        todo->to_pid = process_array[i].pid;
                        todo->response = -1;
                        process_message *copy = (process_message *)malloc(sizeof(process_message));
                        *copy = *todo;
                        copy->content_address = -1;
                        process_array[i].should_be_active = true;
                        return copy;
                    }
                } // every left message has been checked
                for (int j = 0; j < MAX_PROCESS_MESSAGE_QUEUE; j++)
                {
                    if (process_array[i].msg_list[j].entry_free_to_use == true)
                    {
                        process_array[i].last_message_used = 0;
                        process_message *todo = &process_array[i].msg_list[j];
                        todo->entry_free_to_use = false;
                        todo->has_been_readed = false;
                        todo->content_length = data_length;
                        uint64_t memory_kernel_copy = (uint64_t)malloc(data_length);
                        memcpy((void *)memory_kernel_copy, (void *)data_addr, data_length);
                        todo->content_address = memory_kernel_copy;
                        todo->from_pid = get_current_data()->current_process->pid;
                        todo->to_pid = process_array[i].pid;
                        todo->response = -1;
                        process_message *copy = (process_message *)malloc(sizeof(process_message));
                        *copy = *todo;
                        copy->content_address = -1;
                        process_array[i].should_be_active = true;
                        return copy;
                    }
                }
            }
        }
    }
    log("process", LOG_ERROR) << "trying to send a message to : " << to_process << "and not founded it :(";

    return nullptr;
}

process_message *read_message()
{
    for (uint64_t i = 0; i < MAX_PROCESS_MESSAGE_QUEUE; i++)
    {
        if (get_current_data()->current_process->msg_list[i].entry_free_to_use == false)
        {
            if (get_current_data()->current_process->msg_list[i].has_been_readed == false)
            {
                return &get_current_data()->current_process->msg_list[i];
            }
        }
    }
    return 0x0;
}
uint64_t message_response(process_message *message_id)
{

    if (message_id->to_pid > MAX_PROCESS)
    {
        //      log("process", LOG_ERROR) << "not valid pid in message response" << message_id->to_pid;
        return -1;
    }
    if (message_id->from_pid != get_current_data()->current_process->pid)
    {
        log("process", LOG_ERROR) << "not valid process from";
        return -1;
    }
    if (process_array[message_id->to_pid].current_process_state != PROCESS_WAITING && process_array[message_id->to_pid].current_process_state != PROCESS_RUNNING)
    {
        log("process", LOG_ERROR) << "trying to send a message to a not started pid" << message_id->to_pid;
        return -1;
    }

    process_message *msg = &process_array[message_id->to_pid].msg_list[message_id->message_id];
    if (msg->response == -1)
    {
        //     log("process", LOG_ERROR) << "message has not been readed";
        return -2; // return -2 = wait again
    }
    else if (msg->entry_free_to_use == true)
    {
        log("process", LOG_ERROR) << "message has already been readed";
        return -3;
    }
    else
    {
        msg->entry_free_to_use = true;
        free(message_id);
        free((void *)msg->content_address);
        return msg->response;
    }
    return -1;
}
void set_on_request_service(bool is_ORS)
{
    if (is_ORS == true)
    {
        log("process", LOG_INFO) << "setting process : " << get_current_data()->current_process->process_name << "on request service mode ";
    }
    get_current_data()->current_process->is_ORS = is_ORS;
    get_current_data()->current_process->should_be_active = false;
}
void on_request_service_update()
{
    get_current_data()->current_process->should_be_active = false;
}
