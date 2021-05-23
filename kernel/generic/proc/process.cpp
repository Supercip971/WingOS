#include <64bit.h>
#include <kernel.h>
#include <proc/process.h>
#include <stdlib.h>
#include <syscall.h>
#include <utility.h>
#include <utils/attribute.h>
#include <utils/lock.h>
#include <utils/memory/liballoc.h>
// when a process is dead but we still want to know some things we use this
struct last_sign_of_process_status
{
    pid_t pid;
    uint32_t status;
    uint32_t ret;
};
utils::vector<last_sign_of_process_status> dead_process_status;
process **process_array = nullptr;
int process_locked = 1;
bool process_loaded = false;

utils::lock_type process_creator_lock;
uint64_t last_process = 0;
process *current_cpu_process[MAX_CPU_COUNT] = {0};
utils::lock_type task_lock;
int dying_process_count = 0;
uint64_t next_upid = 1;

process *process::current()
{
    return current_cpu_process[get_current_cpu_id()];
}
void process::set_current(process *target)
{
    current_cpu_process[get_current_cpu_id()] = target;
}

void lock_process()
{
    process_locked++;
}

void unlock_process()
{
    process_locked--;
}

int is_process_locked()
{
    return process_locked != 0;
}
NO_RETURN void null_process()
{
    turn_on_interrupt();

    while (true)
    {
        yield();
    }
}

bool process::destroy()
{

    set_active(false);
    set_state(PROCESS_AVAILABLE);
    return true;
}

NO_RETURN void utility_process()
{
    turn_on_interrupt();

    while (true)
    {
        while (dying_process_count == 0)
        {
            sleep(100);
        }
        for (uint64_t i = 0; i < MAX_PROCESS; i++)
        {
            if ((process_loaded == true && i == 0) || process_array[i] == nullptr) // process 0 is null only after the kernel start
            {
                continue;
            }
            process_creator_lock.lock();
            lock_process();
            if (process_array[i]->get_state() == PROCESS_SHOULD_BE_DEAD)
            {

                log("proc", LOG_INFO, "killing process [{}] : {}", i, process_array[i]->get_name());

                process_array[i]->destroy();
                process_array[i] = nullptr;
                dying_process_count--;
                process_creator_lock.unlock();
            }
            unlock_process();
        }
        sleep(100);
    }
}

NO_RETURN void init_multi_process(func start)
{
    log("proc", LOG_DEBUG, "loading multi processing");

    process_array = reinterpret_cast<process **>(malloc(sizeof(process *) * MAX_PROCESS + PAGE_SIZE));
    process::set_current(nullptr);

    for (size_t i = 0; i < MAX_PROCESS; i++)
    {
        process_array[i] = nullptr;
    }

    init_process(null_process, true, "testing1", false);

    init_process(start, true, "kernel process", false);

    log("proc", LOG_INFO, "loading smp process for cpus: {}", get_cpu_count());

    for (size_t i = 0; i <= get_cpu_count(); i++) // init 2
    {
        init_process(null_process, true, "smp1", false, i);
        init_process(null_process, true, "smp2", false, i);
    }

    init_process(utility_process, true, "kproc", false, 0);

    process_loaded = true;

    unlock_process();

    turn_on_interrupt();
    process_locked = 0;
    while (true)
    {
        halt_interrupt();
    }
}

int64_t find_free_process()
{
    for (uint64_t i = last_process; i < MAX_PROCESS; i++)
    {
        if ((process_loaded == true && i == 0) || process_array[i] != nullptr) // process 0 is null only after the kernel start
        {
            continue;
        }
        if (process_array[i] == nullptr)
        {
            last_process = i + 1;
            return i;
        }
    }
    if (last_process == 0)
    {
        return -1;
    }
    else
    {
        last_process = 0;
        return find_free_process();
    }
}

process *alloc_process(const char *end_name, func entry_point, bool is_user)
{
    int64_t process_to_add_kpid = find_free_process();
    if (process_to_add_kpid == -1)
    {
        log("process", LOG_ERROR, "no free process founded");
        return nullptr;
    }
    next_upid++;
    process_array[process_to_add_kpid] = new process(end_name, process_to_add_kpid, next_upid, reinterpret_cast<uintptr_t>(entry_point), is_user);
    process_array[process_to_add_kpid]->set_state(process_state::PROCESS_NOT_STARTED);
    return process_array[process_to_add_kpid];
}

void init_process_entry(process *target, const char *name, bool is_user, char **argv, int argc, func entry_point)
{
    init_process_stackframe(target, entry_point, argc, argv);
    init_process_userspace_fs(target->get_ufs());
    init_process_paging(target, is_user);
    init_process_arch_ext(target);
}

process *init_process(func entry_point, bool start_direct, const char *name, bool user, uint64_t cpu_target, int argc, char **argv)
{
    utils::context_lock locker(process_creator_lock);

    auto process_to_add = alloc_process(name, entry_point, user);

    if (process_to_add == nullptr)
    {
        log("process", LOG_ERROR, "can't allocate process");
        return nullptr;
    }

    log("process", LOG_INFO, "adding process: {} entry: {} name: {}", process_to_add->get_pid(), reinterpret_cast<uintptr_t>(entry_point), process_to_add->get_name());

    process_to_add->set_cpu(interpret_cpu_request(cpu_target));
    init_process_entry(process_to_add, name, user, argv, argc, entry_point);

    process_to_add->set_parent(process_to_add->get_pid());

    if (start_direct || process::current() == nullptr)
    {
        process_to_add->set_state(process_state::PROCESS_WAITING);
        process::set_current(process_to_add);
    }

    return process_to_add;
}

process *get_next_process(uint64_t current_id)
{
    if (process_locked != 0)
    {
        return process::current();
    }

    uint64_t dad = get_current_cpu_id();
    for (uint64_t i = current_id + 1; i < MAX_PROCESS; i++)
    {

        if (process_array[i] != nullptr && process_array[i]->is_runnable(dad))
        {

            return process_array[i];
        }
    }

    if (current_id != 0)
    {
        return get_next_process(0);
    }

    log("proc", LOG_ERROR, "no process found");
    log("proc", LOG_ERROR, "from cpu: {}", get_current_cpu_id());
    log("proc", LOG_ERROR, "maybe from: {}", process::current()->get_name());

    dump_process();
    while (true)
    {
    }
    return nullptr;
}

uintptr_t process_switch_handler(arch_stackframe *isf, bool switch_all) // switch all is true only for pit/hpet/... interrupt
{

    if (process_locked != 0)
    {
        return (uintptr_t)isf;
    }
    if (switch_all)
    {
        for (uint64_t i = 1; i < MAX_PROCESS; i++)
        {

            if (process_array[i] != nullptr && process_array[i]->is_sleeping())
            {
                process_array[i]->decrease_sleep(1);
            }
        }
        send_switch_process_to_all_cpu();
    }
    process *i = nullptr;

    if (process::current() == NULL)
    {
        i = get_next_process(0);
    }
    else
    {
        i = get_next_process(process::current()->get_kpid());
    }

    if (i == 0)
    {
        return (uint64_t)isf;
    }

    return switch_context(isf, i);
}

process *process::from_name(const char *name)
{
    for (size_t i = 0; i < MAX_PROCESS; i++)
    {
        if (process_array[i] != nullptr)
        {
            if (strcmp(name, process_array[i]->get_name()) == 0)
            {
                return process_array[i];
            }
        }
    }
    return nullptr;
}

process *process::from_pid(pid_t pid)
{

    for (size_t i = 0; i < MAX_PROCESS; i++)
    {
        if (process_array[i] != nullptr)
        {
            if (process_array[i]->get_pid() == pid)
            {
                return process_array[i];
            }
        }
    }
    return nullptr;
}

void rename_process(const char *name, pid_t pid)
{

    process *target = process::from_pid(pid);
    log("process", LOG_INFO, "renamming process: {} to: {}", target->get_name(), name);

    lock_process();
    target->rename(name);
    unlock_process();
}

void sleep(uint64_t count)
{
    lock_process();
    process::current()->increase_sleep(count);
    unlock_process();
    yield();
}

void sleep(uint64_t count, pid_t pid)
{
    lock_process();

    process::from_pid(pid)->increase_sleep(count);
    unlock_process();
}

void dump_process()
{
    lock_process();
    for (int i = 0; i < MAX_PROCESS; i++)
    {
        if (process_array[i] != nullptr)
        {
            log("proc", LOG_DEBUG, "info for process: {}", i);
            log("proc", LOG_INFO, "process name : {}", process_array[i]->get_name());
            log("proc", LOG_INFO, "process state: {}", (int)process_array[i]->get_state());
            log("proc", LOG_INFO, "process cpu  : {}", process_array[i]->get_cpu());
            log("proc", LOG_INFO, "process upid : {}", process_array[i]->get_pid());
            log("proc", LOG_INFO, "process sleep: {}", process_array[i]->get_sleep_count());
        }
    }
    unlock_process();
}

void kill(pid_t pid)
{
    auto target = process::from_pid(pid);
    if (target == nullptr)
    {

        log("proc", LOG_ERROR, "can't kill process {} ", pid);
        return;
    }
    lock_process();
    target->kill();
    dying_process_count++;
    unlock_process();
}

void add_current_process_as_dead(int code)
{

    last_sign_of_process_status s;
    s.pid = process::current()->get_pid();
    s.ret = code;
    s.status = 1;
    dead_process_status.push_back(s);
}

NO_RETURN void kill_current(int code)
{
    lock_process();

    asm volatile("cli");

    log("proc", LOG_INFO, "killing current process");
    add_current_process_as_dead(code);
    process::current()->kill();
    dying_process_count++;
    unlock_process();
    asm volatile("sti");
    while (true)
    {

        yield();
    }
}

uintptr_t process::allocate_virtual_addr(size_t count)
{
    auto ret = virtual_addr.alloc(count);

    return ret;
}

uintptr_t process::free_virtual_addr(uintptr_t addr, size_t count)
{
    if (has_virtual_addr(addr, count))
    {
        virtual_addr.set_free(addr, count);
        return count;
    }
    else
    {
        log("process", LOG_ERROR, "can't free virtual addr: {} count: {}", addr * PAGE_SIZE, count);
        return 0;
    }
}

bool process::has_virtual_addr(uintptr_t addr, size_t count)
{
    if (((addr + count) * PAGE_SIZE) > (USR_VIRT_ADDR_SIZE))
    {
        return false;
    }

    for (size_t i = addr; i < addr + count; i++)
    {
        if (virtual_addr.get(i) != true)
        {
            return false;
        }
    }
    return true;
}

void get_process_status(pid_t pid, int *ret, int *status)
{
    for (size_t i = 0; i < dead_process_status.size(); i++)
    {
        if (dead_process_status[i].pid == pid)
        {
            *ret = dead_process_status[i].ret;
            *status = -2;
            return;
        }
    }

    auto proc = process::from_pid(pid);
    if (proc == nullptr)
    {
        log("process", LOG_ERROR, "invalid pid: {} for {}", pid, __FUNCTION__);
    }
    *status = proc->get_state() != PROCESS_RUNNING;
    *ret = -250;
}
