#include <arch/64bit.h>
#include <arch/gdt.h>
#include <arch/lock.h>
#include <arch/mem/memory_manager.h>

#include <arch/mem/memory_manager.h>
#include <arch/process.h>
#include <arch/smp.h>
#include <arch/sse.h>
#include <com.h>
#include <device/apic.h>
#include <device/local_data.h>
#include <kernel.h>
#include <logging.h>
#include <syscall.h>
#include <utility.h>

process kernel_process;
bool cpu_wait = false;
char temp_esp[8192];
process *nxt;

int process_locked = 1;
bool process_loaded = false;

lock_type process_creator_lock = {0};
uint64_t last_process = 0;
uint8_t proc_last_selected_cpu = 0;

lock_type task_lock = {0};
lock_type lck_lck = {0};

uint64_t next_upid = 1;

void lock_process()
{
    process_locked++;
}

void unlock_process()
{
    process_locked--;
}

extern "C" void irq0_first_jump();
extern "C" void reload_cr3();

extern "C" void start_task()
{
    lock(&task_lock);
}

extern "C" void end_task()
{
    unlock(&task_lock);
}

void null_process()
{
    asm("sti");

    while (true)
    {
        yield();
    }
}

extern "C" void irq0_first_jump(void);
void init_multi_process(func start)
{
    log("proc", LOG_DEBUG) << "loading multi processing";

    process_array = (process *)((uint64_t)malloc(sizeof(process) * MAX_PROCESS + 4096 /* for security */));

    get_current_cpu()->current_process = nullptr;

    for (size_t i = 0; i < MAX_PROCESS; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            process_array[i].pr_buff[j].data = nullptr;
        }
        process_array[i].kpid = i;
        process_array[i].upid = -1;
        process_array[i].current_process_state = process_state::PROCESS_AVAILABLE;
        memzero(process_array[i].stack, PROCESS_STACK_SIZE);
    }

    init_process(null_process, true, "testing1", false);

    init_process(start, true, "kernel process", false);

    log("proc", LOG_INFO) << "loading smp process for cpus :" << smp::the()->processor_count;

    for (size_t i = 0; i <= smp::the()->processor_count; i++)
    {
        init_process(null_process, true, "smp1", false, i);
        init_process(null_process, true, "smp2", false, i);
    }

    process_loaded = true;

    unlock_process();

    asm volatile("sti");

    process_locked = 0;
    while (true)
    {
        asm volatile("hlt");
    }
}

uint64_t interpret_cpu_request(uint64_t cpu)
{
    if (cpu == -1)
    {
        return apic::the()->get_current_processor_id();
    }
    else if (cpu == -2)
    {
        proc_last_selected_cpu++;

        if (proc_last_selected_cpu > smp::the()->processor_count)
        {
            proc_last_selected_cpu = 0;
        }

        cpu = proc_last_selected_cpu;
    }
    return cpu;
}

void init_process_stackframe(process *pro, func entry_point)
{
    memzero(pro->stack, PROCESS_STACK_SIZE);
    pro->rsp =
        ((uint64_t)pro->stack) + PROCESS_STACK_SIZE;

    uint64_t *rsp = (uint64_t *)pro->rsp;

    InterruptStackFrame *ISF = (InterruptStackFrame *)(rsp - (sizeof(InterruptStackFrame)));

    ISF->rip = (uint64_t)entry_point;
    ISF->ss = SLTR_KERNEL_DATA;
    ISF->cs = SLTR_KERNEL_CODE;
    ISF->rflags = 0x286;
    ISF->rsp = (uint64_t)ISF;
    pro->rsp = (uint64_t)ISF;
}

process *find_usable_process()
{
    for (uint64_t i = last_process; i < MAX_PROCESS; i++)
    {
        if (process_loaded == true && i == 0) // process 0 is null only after the kernel start
        {
            continue;
        }
        if (process_array[i].current_process_state ==
            process_state::PROCESS_AVAILABLE)
        {
            last_process = i + 1;
            process_array[i].current_process_state = process_state::PROCESS_NOT_STARTED;
            return &process_array[i];
        }
    }
    if (last_process == 0)
    {
        return nullptr;
    }
    else
    {
        last_process = 0;
        return find_usable_process();
    }
}

void init_process_global_memory(process *to_init)
{
    to_init->global_process_memory = (uint8_t *)((uint64_t)malloc(4096));
    to_init->global_process_memory_length = 4096;
    memzero(to_init->global_process_memory, to_init->global_process_memory_length);
}

void init_process_message(process *to_init)
{
    to_init->last_message_used = 0;

    for (uint64_t j = 0; j < MAX_PROCESS_MESSAGE_QUEUE; j++)
    {
        to_init->msg_list[j].entry_free_to_use = true;
        to_init->msg_list[j].message_id = j;
    }
}

void init_process_buffers(process *to_init)
{

    to_init->pr_buff[0].type = STDOUT;
    to_init->pr_buff[1].type = STDIN;
    to_init->pr_buff[2].type = STDERR;
    for (int i = 0; i < 3; i++)
    {
        to_init->pr_buff[i].allocated_length = 128;
        if (to_init->pr_buff[i].data != nullptr)
        {
            free((void *)get_rmem_addr((uint64_t)to_init->pr_buff[i].data));
        }
        to_init->pr_buff[i].length = 0;
        to_init->pr_buff[i].data = (uint8_t *)get_mem_addr((uint64_t)malloc(128));
    }
}

process *init_process(func entry_point, bool start_direct, const char *name, bool user, int cpu_target)
{
    lock((&process_creator_lock));
    process *process_to_add = find_usable_process();
    if (process_to_add == nullptr)
    {
        log("proc", LOG_ERROR) << "init_process : no free process found";
    }

    bool added_pid = false;

    if (get_pid_from_process_name(name) != -1)
    {
        log("proc", LOG_INFO) << "process with name already exist so add pid at the end";
        added_pid = true;
    }
    process_to_add->upid = next_upid;
    log("proc", LOG_INFO) << "adding process" << process_to_add->upid << "entry : " << (uint64_t)entry_point << "name : " << name;
    next_upid++;
    if (start_direct == true)
    {
        process_to_add->current_process_state = process_state::PROCESS_WAITING;
    }

    memcpy(process_to_add->process_name, name, strlen(name) + 2);
    if (added_pid)
    {
        process_to_add->process_name[strlen(name) + 1] = process_to_add->upid;
    }

    init_process_global_memory(process_to_add);
    init_process_message(process_to_add);
    init_process_stackframe(process_to_add, entry_point);
    init_process_buffers(process_to_add);

    process_to_add->entry_point = (uint64_t)entry_point;

    process_to_add->processor_target = interpret_cpu_request(cpu_target);

    process_to_add->is_ORS = false;

    process_to_add->sleeping = 0;
    if (user)
    {
        process_to_add->page_directory = (uint64_t)new_vmm_page_dir();
    }
    else
    {
        process_to_add->page_directory = (uint64_t)get_current_cpu()->page_table;
    }

    if (get_current_cpu()->current_process == 0x0)
    {
        get_current_cpu()->current_process = process_to_add;
    }

    unlock((&process_creator_lock));

    return process_to_add;
}

extern "C" uint64_t switch_context(InterruptStackFrame *current_Isf, process *next)
{

    if (process_locked != 0)
    {
        return (uint64_t)current_Isf; // early return
    }

    if (next == NULL)
    {
        if (cpu_wait)
        {
            cpu_wait = false;
        }

        return (uint64_t)current_Isf; // early return
    }

    if (get_current_cpu()->current_process != NULL)
    {
        get_current_cpu()->current_process->current_process_state = PROCESS_WAITING;
        get_current_cpu()->current_process->rsp = (uint64_t)current_Isf;

        get_current_cpu()->save_sse(get_current_cpu()->current_process->sse_context);
    }
    next->current_process_state = process_state::PROCESS_RUNNING;
    get_current_cpu()->current_process = next;
    get_current_cpu()->load_sse(get_current_cpu()->current_process->sse_context);
    task_update_switch(next);

    if (cpu_wait)
    {
        cpu_wait = false;
    }
    return next->rsp;
}
bool is_valid_process(const process target, const int cpu_targ)
{
    return ((target.current_process_state ==
             process_state::PROCESS_WAITING) &&
            target.kpid != 0 && target.processor_target == cpu_targ);
}
extern "C" process *get_next_process(uint64_t current_id)
{
    if (process_locked != 0)
    {
        return get_current_cpu()->current_process;
    }

    uint64_t dad = apic::the()->get_current_processor_id();
    for (uint64_t i = current_id + 1; i < MAX_PROCESS; i++)
    {

        if (is_valid_process(process_array[i], dad))
        {

            if (process_array[i].is_ORS == true && process_array[i].should_be_active == false)
            {
                continue;
            }
            if (process_array[i].sleeping != 0)
            {
                continue;
            }
            return &process_array[i];
        }
    }

    if (current_id != 0)
    {
        return get_next_process(0);
    }

    log("proc", LOG_ERROR) << "no process found";
    log("proc", LOG_ERROR) << "from cpu : " << apic::the()->get_current_processor_id();
    log("proc", LOG_ERROR) << "maybe from : " << get_current_cpu()->current_process->process_name;

    dump_process();
    return get_current_cpu()->current_process;
}

void send_switch_process_to_all_cpu()
{
    if (apic::the()->get_current_processor_id() == 0)
    {
        for (int i = 0; i <= smp::the()->processor_count; i++)
        {
            if (i != apic::the()->get_current_processor_id())
            {
                apic::the()->send_ipi(i, 100);
            }
        }
    }
}
extern "C" uint64_t irq_0_process_handler(InterruptStackFrame *isf)
{

    if (process_locked != 0)
    {
        return (uint64_t)isf;
    }
    if (isf->int_no != 100)
    {
        for (uint64_t i = 0; i < MAX_PROCESS; i++)
        {

            if ((process_array[i].current_process_state ==
                 process_state::PROCESS_WAITING) &&
                process_array[i].kpid != 0 && process_array[i].sleeping != 0)
            {

                process_array[i].sleeping--;
            }
        }
        send_switch_process_to_all_cpu();
    }
    process *i = nullptr;

    if (get_current_cpu()->current_process == NULL)
    {
        i = get_next_process(0);
    }
    else
    {
        i = get_next_process(get_current_cpu()->current_process->kpid);
    }

    if (i == 0)
    {
        return (uint64_t)isf;
    }

    return switch_context(isf, i);
}

extern "C" void task_update_switch(process *next)
{
    tss_set_rsp0((uint64_t)next->stack + PROCESS_STACK_SIZE);

    get_current_cpu()->page_table = (uint64_t *)next->page_directory;
    update_paging();
}

void add_thread_map(process *p, uint64_t from, uint64_t to, uint64_t length)
{
    for (uint64_t i = 0; i < length; i++)
    {
        map_page((main_page_table *)p->page_directory, from + i * PAGE_SIZE, to + i * PAGE_SIZE, PAGE_TABLE_FLAGS);
    }
}

uint64_t get_pid_from_process_name(const char *name)
{
    for (size_t i = 0; i < MAX_PROCESS; i++)
    {
        if (process_array[i].current_process_state == PROCESS_WAITING || process_array[i].current_process_state == PROCESS_RUNNING)
        {
            if (strcmp(name, process_array[i].process_name) == 0)
            {
                return process_array[i].upid;
            }
        }
    }
    return -1;
}

process_message *get_new_process_message(process *target)
{
    for (int j = target->last_message_used; j < MAX_PROCESS_MESSAGE_QUEUE; j++)
    {
        if (target->msg_list[j].entry_free_to_use == true)
        {
            process_message *msg = &target->msg_list[j];
            msg->entry_free_to_use = false;
            msg->has_been_readed = false;
            msg->response = -1;
            target->last_message_used = j;
            return msg;
        }
    }

    // if no free entry founded reloop
    if (target->last_message_used != 0)
    {
        target->last_message_used = 0;
        return get_new_process_message(target);
    }
    return nullptr;
}

process_message *create_process_message(size_t tpid, uint64_t data_addr, uint64_t data_length)
{

    uint64_t target_kpid = upid_to_kpid(tpid);
    process_message *todo = get_new_process_message(&process_array[target_kpid]);
    if (todo == nullptr)
    {
        log("proc", LOG_ERROR) << "can't create a process message";
        return nullptr;
    }
    todo->content_length = data_length;

    uint64_t memory_kernel_copy = (uint64_t)malloc(data_length);
    memcpy((void *)memory_kernel_copy, (void *)data_addr, data_length);
    todo->content_address = memory_kernel_copy;

    todo->from_pid = get_current_cpu()->current_process->upid;
    todo->to_pid = tpid;

    process_message *copy = (process_message *)malloc(sizeof(process_message));
    *copy = *todo;
    copy->content_address = -1;
    return copy;
}

process_message *send_message(uint64_t data_addr, uint64_t data_length, const char *to_process)
{
    for (int i = 0; i < MAX_PROCESS; i++)
    {
        if (process_array[i].current_process_state == PROCESS_WAITING || process_array[i].current_process_state == PROCESS_RUNNING)
        {
            if (strcmp(to_process, process_array[i].process_name) == 0)
            {
                process_array[i].should_be_active = true;
                return create_process_message(process_array[i].upid, data_addr, data_length);
            }
        }
    }

    log("process", LOG_ERROR) << "trying to send a message to : " << to_process << "and not founded it :(";
    return nullptr;
}

void dump_process()
{
    lock_process();

    for (int i = 0; i < MAX_PROCESS; i++)
    {
        if (process_array[i].current_process_state != PROCESS_AVAILABLE)
        {
            log("proc", LOG_DEBUG) << "info for process : " << i;
            log("proc", LOG_INFO) << "process name     : " << process_array[i].process_name;
            log("proc", LOG_INFO) << "process state    : " << process_array[i].current_process_state;
            log("proc", LOG_INFO) << "process cpu      : " << process_array[i].processor_target;
            log("proc", LOG_INFO) << "process upid     : " << process_array[i].upid;
        }
    }

    unlock_process();
}

process_message *read_message()
{
    for (uint64_t i = 0; i < MAX_PROCESS_MESSAGE_QUEUE; i++)
    {
        if (get_current_cpu()->current_process->msg_list[i].entry_free_to_use == false)
        {
            if (get_current_cpu()->current_process->msg_list[i].has_been_readed == false)
            {
                return &get_current_cpu()->current_process->msg_list[i];
            }
        }
    }

    if (get_current_cpu()->current_process->is_ORS == true)
    {
        get_current_cpu()->current_process->should_be_active = false;
    }

    return 0x0;
}

uint64_t message_response(process_message *message_id)
{

    if (message_id->from_pid != get_current_cpu()->current_process->upid)
    {
        log("process", LOG_ERROR) << "not valid process from" << message_id->from_pid;
        return -1;
    }

    uint64_t target_kpid = upid_to_kpid(message_id->to_pid);
    if (process_array[target_kpid].current_process_state != PROCESS_WAITING && process_array[target_kpid].current_process_state != PROCESS_RUNNING)
    {
        log("process", LOG_ERROR) << "trying to send a message to a not started pid" << message_id->to_pid;
        return -1;
    }

    process_message *msg = &process_array[target_kpid].msg_list[message_id->message_id];

    if (msg->has_been_readed == false)
    {
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

    get_current_cpu()->current_process->is_ORS = is_ORS;

    get_current_cpu()->current_process->should_be_active = true;
}

void set_on_request_service(bool is_ORS, uint64_t pid)
{

    uint64_t kpid = upid_to_kpid(pid);
    process_array[kpid].is_ORS = is_ORS;
    process_array[kpid].should_be_active = true;
}

void on_request_service_update()
{
    get_current_cpu()->current_process->should_be_active = false;
}

uint64_t get_process_global_data_copy(uint64_t offset, const char *process_name)
{
    uint64_t pid = get_pid_from_process_name(process_name);

    uint64_t kpid = upid_to_kpid(pid);
    if (kpid == -1)
    {
        log("process", LOG_ERROR) << "get global data copy, trying to get a non existant process : " << process_name;
        return -1;
    }
    if (process_array[kpid].global_process_memory_length < offset + sizeof(uint64_t))
    {
        log("process", LOG_ERROR) << "getting out of range process data";
        return -1;
    }
    else
    {
        uint8_t *data_from = process_array[kpid].global_process_memory;

        return *((uint64_t *)(data_from + offset));
    }
}

void *get_current_process_global_data(uint64_t offset, uint64_t length)
{
    if (get_current_cpu()->current_process->global_process_memory_length < offset + length)
    {
        log("process", LOG_ERROR) << "getting out of range process data";
        return nullptr;
    }
    else
    {
        return get_current_cpu()->current_process->global_process_memory + offset;
    }
}

void set_on_interrupt_process(uint8_t added_int)
{
    get_current_cpu()->current_process->is_on_interrupt_process = true;

    for (int i = 0; i < 8; i++)
    {
        if (get_current_cpu()->current_process->interrupt_handle_list[i] == 0)
        {
            get_current_cpu()->current_process->interrupt_handle_list[i] = added_int;
            return;
        }
    }

    log("process", LOG_ERROR) << "no free interrupt entry for process", get_current_cpu()->current_process->process_name;
}
uint64_t upid_to_kpid(uint64_t upid)
{
    for (int i = 0; i < MAX_PROCESS; i++)
    {

        if (process_array[i].upid == upid)
        {
            return process_array[i].kpid;
        }
    }
    log("process", LOG_ERROR) << "not founded process with upid : " << upid;
    return -1;
}
void rename_process(const char *name, uint64_t pid)
{

    uint64_t kpid = upid_to_kpid(pid);
    log("process", LOG_INFO) << "renamming process: " << process_array[kpid].process_name << " to : " << name;

    lock(&lck_syscall); // turn off syscall
    lock_process();
    memcpy(process_array[kpid].backed_name, process_array[kpid].process_name, 128);
    memzero(process_array[kpid].process_name, 128);
    memcpy(process_array[kpid].process_name, name, strlen(name) + 1);

    unlock_process();
    unlock(&lck_syscall);
}

bool add_process_buffer(process_buffer *buf, uint64_t data_length, uint8_t *raw)
{
    if ((buf->length + data_length) > buf->allocated_length)
    {
        // never gonna cast you up
        // never gonna cast you doooooown
        buf->allocated_length += 512;
        uint8_t *new_buffer = (uint8_t *)realloc((void *)((uint64_t)buf->data), buf->allocated_length);
        buf->data = (uint8_t *)((uint64_t)new_buffer);
    }

    memcpy(buf->data + buf->length, raw, data_length);
    buf->length += data_length;
    return true;
}

void sleep(uint64_t count)
{
    get_current_cpu()->current_process->sleeping += count;
    yield();
}
