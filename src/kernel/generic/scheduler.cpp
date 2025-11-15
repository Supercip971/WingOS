#include "kernel/generic/scheduler.hpp"

#include "kernel/generic/cpu_tree.hpp"
#include "libcore/ds/linked_list.hpp"

#include "kernel/generic/context.hpp"
#include "kernel/generic/cpu.hpp"
#include "kernel/generic/paging.hpp"
#include "kernel/generic/task.hpp"
#include "libcore/atomic.hpp"
#include "libcore/ds/array.hpp"
#include "libcore/ds/vec.hpp"
#include "libcore/fmt/log.hpp"
#include "libcore/lock/rwlock.hpp"
#include "libcore/result.hpp"
#include "libcore/type-utils.hpp"

using TaskQueue = core::LinkedList<kernel::Task*>;

core::Vec<size_t> _choosen = {};
core::Vec<size_t> _retried = {};

core::Vec<kernel::Task *> scheduler_idles = {};
core::Vec<kernel::Task*> cpu_runned = {};
core::Vec<kernel::Task*> cpu_running = {};



core::Array<TaskQueue, kernel::TASK_QUEUE_COUNT> task_queues = {};
TaskQueue blocked_tasks = {};

core::RWLock scheduler_lock = {};

size_t running_cpu_count = 0;

bool is_scheduler_running = false;

size_t tick = 0;

namespace kernel
{
static inline void add_entity_to_queue(Task* task)
{

    if (!task->should_run())
    {
        return;
    }

    if (task->sched().block_event.type != BlockEvent::Type::NONE)
    {
        blocked_tasks.push(task);
        return;
    }
    task_queues[task->sched().queue()].push(task);
}


void idle()
{
    while (true)
    {


        asm volatile("sti");
        
        asm volatile("hlt");
    }
}

core::Result<void> scheduler_init_idle_task()
{
    for (size_t i = 0; i < Cpu::count(); i++)
    {
        auto task = Task::task_create();
        if (task.is_error())
        {
            return "unable to create idle task";
        }
        auto t = task.unwrap();

        CpuContextLaunch launch = {};
        launch.entry = (void *)idle;
        launch.user = false;

        t->_initialize(launch, &VmmSpace::kernel_page_table()).assert();
        t->sched() = SchedulerControlBlock(0);
        t->sched().is_idle = true;
        scheduler_idles.push(t);
        

        cpu_runned.push(t);
        cpu_running.push(t);
        //   running_cpu_count++;
    }
    return {};
}

core::Result<void> scheduler_init(int cpu_count)
{
    running_cpu_count = cpu_count;

    if ((size_t)cpu_count != Cpu::count())
    {
        return "cpu count mismatch";
    }
    cpu_runned = {};
    scheduler_lock.write_acquire();

    // Initialize task queues
    for (size_t i = 0; i < kernel::TASK_QUEUE_COUNT; i++)
    {
        task_queues[i] = TaskQueue();
    }

    // Ensure cpu_runned has enough space
    try$(cpu_runned.reserve(running_cpu_count));
    try$(cpu_running.reserve(running_cpu_count));
    cpu_runned.clear();
    cpu_running.clear();


    try$(scheduler_init_idle_task());
    scheduler_lock.write_release();
    for (size_t i = 0; i < running_cpu_count; i++)
    {
        Cpu::get(i)->currentTask(nullptr);
        
        cpu_runned[i] = scheduler_idles[i];
        cpu_runned[i]->sched().is_idle = true;
        cpu_running[i] = cpu_runned[i];
    }
    _choosen = try$(core::Vec<size_t>::with_capacity(running_cpu_count));
    _retried = try$(core::Vec<size_t>::with_capacity(running_cpu_count));

    return {};
}

core::Result<void> scheduler_start()
{
    return {};
}

core::Result<void> scheduler_tick()
{
    return {};
}

core::Result<kernel::Task *> next_task_select(CoreId core)
{
    if ((size_t)core >= running_cpu_count)
    {
        return core::Result<kernel::Task *>::error("invalid core id");
    }

    if (cpu_runned[core] == nullptr)
    {
        if ((size_t)core < scheduler_idles.len())
        {
            return scheduler_idles[core];
        }
        return core::Result<kernel::Task *>::error("no idle task available for core");
    }

    return cpu_runned[core];
}

core::Result<void> task_run(TUID task_id, CoreId core)
{
    (void)core;
    auto task = Task::by_id(task_id);
    if (task.is_error())
    {
        return "unable to find task";
    }

    auto t = task.unwrap();

    t->sched() = SchedulerControlBlock(0);

    t->state(TaskState::TASK_RUNNING);

    scheduler_lock.write_acquire();
    add_entity_to_queue(t);
    scheduler_lock.write_release();
    return {};
}

// FIXME: store it as a global variable instead of recalculating it
static inline size_t scheduled_task_count()
{
    size_t count = 0;
    for (size_t i = 0; i < TASK_QUEUE_COUNT; i++)
    {
        count += task_queues[i].count();
    }

    for (size_t i = 0; i < running_cpu_count; i++)
    {
        if (!cpu_runned[i]->sched().is_idle)
        {
            count++;
        }
    }

    return count;
}

static core::Result<size_t> query_nearest_task(size_t queue_id, CoreId core, bool consider_siblings = false)
{
    // For tasks in the given queue, choose the one whose CPU affinity is either unset,
    // already matching the given CPU, or whose affinity is “close” (i.e. sibling) to the given CPU.

    auto &queue = task_queues[queue_id];
    size_t off = 0;
    for (auto task = queue.begin(); task != queue.end(); ++task)
    {

        auto cpu_affinity = (*task)->sched().cpu_affinity;
        if (cpu_affinity == CpuCoreNone)
        {
            cpu_affinity = core;
            // first time runned with no affinity
            return off;
        }
        // maybe to fix
        if (cpu_affinity == core)
        {
            return off;
        }
        if (consider_siblings && CpuTreeNode::root()->are_they_sibling(cpu_affinity, core))
        {
            return off;
        }
        off++;
    }

    return "no task found";
}

static long summed_weights()
{
    long sum = 0;
    for (size_t i = 0; i < TASK_QUEUE_COUNT; i++)
    {
        for (auto task = task_queues[i].begin(); task != task_queues[i].end(); ++task)
        {
            sum += (*task)->sched().weight();
        }
    }

    return sum;
}
static void update_runned_task_for_cpu(CoreId cpu)
{

    // technically we should be able to do this in one iteration, and this is a bit of a waste
    // but for now, I prefer having readable code rather than optimized one
    // - say the guy who spend too much time optimizing a scheduler algorithm before having a userspace

    auto& sched_block = cpu_runned[cpu]->sched();
    if (sched_block.is_idle)
    {
        return;
    }
    sched_block.running += 1;
    sched_block.old_cpu_affinity = cpu;

    add_entity_to_queue(cpu_runned[cpu]);

    // FIX: Clear the CPU slot after moving task to queue
    cpu_runned[cpu] = scheduler_idles[cpu];
    cpu_runned[cpu]->sched().cpu_affinity = CpuCoreNone;
    cpu_runned[cpu]->sched().old_cpu_affinity = CpuCoreNone;
}

static void update_runned_tasks()
{
    // actually quick because of linked list
    // we shift everything from a queue to the previous one
    // to avoid the need of sorting
    for (size_t i = 0; i < TASK_QUEUE_COUNT - 1; i++)
    {
        if (i == 0)
        {
            task_queues[i] += core::move(task_queues[i + 1]);
        }
        else
        {
            task_queues[i] = core::move(task_queues[i + 1]);
        }
    }

    task_queues[TASK_QUEUE_COUNT - 1].release();

    // technically we should be able to do this in one iteration, and this is a bit of a waste
    // but for now, I prefer having readable code rather than optimized one
    // - say the guy who spend too much time optimizing a scheduler algorithm before having a userspace

    auto sum = summed_weights();
    auto count = scheduled_task_count();

    if (count != 0)
    {

        long avg_sleep_time = (sum + count - 1) / count; // integer ceil (TODO: check)
        for (size_t i = 0; i < TASK_QUEUE_COUNT; i++)
        {
            for (auto task = task_queues[i].begin(); task != task_queues[i].end(); ++task)
            {
                (*task)->sched().sleeping += avg_sleep_time;
            }
        }

        for (auto &task : blocked_tasks)
        {
            task->sched().sleeping += avg_sleep_time;
        }
    }

    for (size_t i = 0; i < running_cpu_count; i++)
    {
        if (cpu_runned[i] == nullptr || cpu_runned[i]->sched().is_idle)
        {
            continue;
        }


        auto& sched_block = cpu_runned[i]->sched();  
        sched_block.running += 1;
        sched_block.old_cpu_affinity = i;
        add_entity_to_queue(cpu_runned[i]);

        // FIX: Clear the CPU slot after moving task to queue
        cpu_runned[i] = scheduler_idles[i];
    }
}

static core::Result<void> fix_sched_affinity()
{
    size_t attempt = 0;
    static core::Vec<CoreId> to_fix = {};

    // technically, should be allocated once
    for (size_t i = 0; i < running_cpu_count; i++)
    {
        to_fix.push(i);
    }

    const size_t max_attempt = 4 * running_cpu_count;

    while (attempt < max_attempt && to_fix.len() > 0)
    {
        attempt++;
        CoreId current = to_fix.pop();
        auto task_associated = cpu_runned[current];
        if (task_associated->sched().is_idle)
        {
            continue;
        }

        auto old_cpu = task_associated->sched().old_cpu_affinity;

        // can't fix a task that is already on a good affinity
        // or a task that is discovering the current cpu
        if (old_cpu == current || old_cpu == CpuCoreNone)
        {
            continue;
        }

        auto stealer = cpu_runned[old_cpu];
        stealer->sched().cpu_affinity = current;

        if (stealer->sched().is_idle)
        {
            cpu_runned[current] = scheduler_idles[current];
        }
        else
        {

            cpu_runned[current] = stealer;
        }

        task_associated->sched().cpu_affinity = old_cpu;
        cpu_runned[old_cpu] = task_associated;

        to_fix.push(old_cpu);
    }

    to_fix.clear();

   // log::log$("fix affinity attempts: {}", attempt);
    if (attempt >= max_attempt)
    {
        log::err$("unable to fix affinity, attempt limit reached");

        return "unable to fix affinity, attempt limit reached";
    }

    return {};
}

void run_task_queued(CoreId cpu, size_t queue_id, size_t queue_offset)
{
    auto task = (task_queues[queue_id].remove(queue_offset));

    if (task == nullptr)
    {
        log::err$("task is null in run_task_queued");
        cpu_runned[cpu] = scheduler_idles[cpu];
        return;
    }
    task->sched().tick();
    task->sched().cpu_affinity = cpu;
    task->sched().is_idle = false;

    cpu_runned[cpu] = (task);
}

core::Result<void> schedule_one(CoreId cpu)
{
    bool found_task = false;
    auto c = scheduled_task_count();
    if (c == 0)
    {
        log::log$("no task to schedule");
        return {};
    }

    for (size_t i = 0; i < TASK_QUEUE_COUNT; i++)
    {
        if (task_queues[i].count() == 0)
        {
            continue;
        }

        // first for the current priority,
        // match the processor with the task with the best affinity
        auto task = query_nearest_task(i, cpu, false);

        if (task.is_error())
        {
            if (!found_task && task_queues[i].count() > 0)
            {
                run_task_queued(cpu, i, 0);
                found_task = true;
            }

            if (found_task)
            {
                break;
            }

            continue;
        }
        auto val = task.unwrap();



        run_task_queued(cpu, i, val);

        found_task = true;
            break;
    }

    if (!found_task)
    {
        cpu_runned[cpu] = scheduler_idles[cpu];
    }
    return {};
}

core::Result<void> schedule_all()
{
    auto *choosen_ptr = &_choosen;
    auto *retried_ptr = &_retried;
    choosen_ptr->clear();
    retried_ptr->clear();
    auto c = scheduled_task_count();
    if (c == 0)
    {
        // log::log$("no task to schedule");
        return {};
    }

    for (size_t i = 0; i < running_cpu_count; i++)
    {
        choosen_ptr->push(i);
    }

    for (size_t i = 0; i < TASK_QUEUE_COUNT; i++)
    {
        retried_ptr->clear(); // make sure retry is fresh for this queue level

        if (task_queues[i].count() == 0)
        {
            continue;
        }

        // first for the current priority,
        // match the processor with the task with the best affinity
        while (choosen_ptr->len() != 0)
        {
            auto cpu = choosen_ptr->pop();
            auto task = query_nearest_task(i, cpu, false);

            if (task.is_error())
            {
                retried_ptr->push(cpu);
                continue;
            }
            auto val = task.unwrap();
            

            run_task_queued(cpu, i, val);
        }

        // Second pass: if tasks remain in the queue, assign them regardless of affinity.
        while (retried_ptr->len() != 0 && task_queues[i].count() > 0)
        {
            auto cpu = retried_ptr->pop();
            run_task_queued(cpu, i, 0);
        }

        // we retry for task with a lower priority by swapping the retried task,

        core::swap(choosen_ptr, retried_ptr);
        retried_ptr->clear();
        
    }

    for (size_t i = 0; i < choosen_ptr->len(); i++)
    {
        auto cpu = (*choosen_ptr)[i];
        cpu_runned[cpu] = scheduler_idles[cpu];
    }
    return {};
}
void schedule_other_cpu(CoreId cpu)
{
    if (Cpu::get(cpu)->currentTask() == nullptr && cpu_runned[cpu] != nullptr)
    {
        cpu_runned[cpu]->cpu_context()->await_load = true;
    }
    else if (Cpu::get(cpu)->currentTask() == nullptr && cpu_runned[cpu] == nullptr)
    {
        return;
    }
    else if (cpu_runned[cpu]->uid() != Cpu::get(cpu)->currentTask()->uid())
    {
        cpu_runned[cpu]->cpu_context()->await_load = true;
        Cpu::get(cpu)->currentTask()->cpu_context()->await_save = true;
    }
}
void schedule_other_cpus()
{
    for (size_t i = 0; i < running_cpu_count; i++)
    {
        if (Cpu::get(i)->currentTask() == nullptr && cpu_runned[i] != nullptr)
        {
            cpu_runned[i]->cpu_context()->await_load = true;
        }
        else if (Cpu::get(i)->currentTask() == nullptr && cpu_runned[i] == nullptr)
        {
            continue;
        }
        else if (cpu_runned[i]->uid() != Cpu::get(i)->currentTask()->uid())
        {
            //  log::log$("Cpu ptr: {}", (uint64_t)cpu_runned[i].task );
            //  log::log$("Current task ptr: {}", (uint64_t)cpu_runned[i].task->cpu_context() );

            cpu_runned[i]->cpu_context()->await_load = true;
            Cpu::get(i)->currentTask()->cpu_context()->await_save = true;
        }
    }

    for (size_t i = 1; i < running_cpu_count; i++)
    {

        if (Cpu::get(i)->currentTask() == nullptr && cpu_runned[i] != nullptr)
        {
            trigger_reschedule(i);
        }
        else if (Cpu::get(i)->currentTask() == nullptr && cpu_runned[i] == nullptr)
        {
            log::err$("cpu {} has no task to run, but is not idle", i);
        }
        else if (cpu_runned[i]->uid() != Cpu::get(i)->currentTask()->uid())
        {
            trigger_reschedule(i);
        }
    }
}

core::Result<void> dump_all_current_running_tasks()
{
    log::log$("Dumping all current running tasks:");
    for (size_t i = 0; i < running_cpu_count; i++)
    {
        if (cpu_runned[i] != nullptr)
        {
            log::log$("CPU[{}] = Task UID: {}, State: {}",
                      i,
                      (int)cpu_runned[i]->uid(),
                      (int)cpu_runned[i]->state());
            log::log$("CPU: ");
            cpu_runned[i]->cpu_context()->dump();
        }
        else
        {
            log::log$("CPU {}: No task running", i);
        }
    }
    return {};
}
core::Result<void> reschedule_only_one(CoreId core)
{
    update_runned_task_for_cpu(core);

    try$(schedule_one(core));

    schedule_other_cpu(core);
    return {};
}

core::Result<void> reschedule_all()
{
    {
        tick++;
        update_runned_tasks();

        try$(schedule_all());

        try$(fix_sched_affinity());
    }
    schedule_other_cpus();

    return {};
}
core::Result<void> resolve_blocked_tasks_scheduler()
{
    for (auto &cpu : cpu_runned)
    {

        if (cpu->sched().block_event.type == BlockEvent::Type::NONE)
        {
            continue;
        }
        if (cpu->sched().block_event.liberated())
        {
            log::log$("unblocking: {}", cpu->uid());

            cpu->sched().block_event = {};
        }
    }
    while (true)
    {

        bool removed = false;
        size_t i = 0;
        for (auto &task : blocked_tasks)
        {
            if (task->sched().block_event.liberated())
            {
                task->sched().block_event = {};

                log::log$("unblocking: {}", task->uid());
                add_entity_to_queue(task);

                blocked_tasks.remove(i);
                removed = true;

                break;
            }
            i++;
        }

        if (!removed)
        {
            break;
        }
    }


    return {};
}
core::Result<Task *> schedule_self(Task *current, void *state, CoreId core)
{
    scheduler_lock.write_acquire();
    resolve_blocked_tasks_scheduler().assert();
    auto v = reschedule_only_one(core);
    if (v.is_error())
    {
        log::err$("unable to reschedule on core {}", core);
        log::err$("we will continue to run the current one");
        scheduler_lock.write_release();
        return {};
    }
 
 
    auto [_next, err] = (next_task_select(core));

    if (err.has_value())
    {
        scheduler_lock.write_release();
        return *err;
    }

    auto next = _next.unwrap();

    if (current != nullptr)
    {
        if (current->uid() == (next)->uid())
        {
            scheduler_lock.write_release();
            return next;
        }
        current->cpu_context()->save_in(state);
    }


    // if the next task is swapped between two cpus, wait
    // for it to save to load it
    

    next->cpu_context()->load_to(state);

    Cpu::current()->currentTask(next);

    cpu_running[Cpu::currentId()] = cpu_runned[Cpu::currentId()];

    scheduler_lock.write_release();
    return next;
 
}

core::Result<Task *> schedule(Task *current, void *state, CoreId core, bool soft)
{

    if (soft)
    {
        return schedule_self(current, state, core);
    }
    else if (core == 0)
    {

        if (!scheduler_lock.try_write_acquire())
        {
            return current;
        }
        auto v = reschedule_all();
        if (v.is_error())
        {
            log::err$("unable to reschedule all tasks");
            log::err$("we will continue to run the current ones");

            scheduler_lock.write_release();
            return {};
        }
        scheduler_lock.write_release();
    }

    scheduler_lock.read_acquire();
    auto [_next, err] = (next_task_select(core));

    if (err.has_value())
    {
        scheduler_lock.read_release();
        return *err;
    }

    auto next = _next.unwrap();

    if (current != nullptr)
    {
        if (current->uid() == (next)->uid())
        {
            scheduler_lock.read_release();
            return next;
        }
        current->cpu_context()->save_in(state);
    }

    scheduler_lock.read_release();

    // if the next task is swapped between two cpus, wait
    // for it to save to load it
    while (true)
    {
        next->cpu_context()->lock.lock();

        if (!next->cpu_context()->await_save)
        {
            next->cpu_context()->lock.release();
            break;
        }

        next->cpu_context()->lock.release();

        asm volatile("pause");
    }

    scheduler_lock.read_acquire();
    next->cpu_context()->load_to(state);

    Cpu::current()->currentTask(next);
    cpu_running[Cpu::currentId()] = cpu_runned[Cpu::currentId()];


    scheduler_lock.read_release();
    return next;
}

core::Result<void> block_current_task(BlockEvent event)
{

    // find the scheduler entity for the current task
    // funny don't need to lock the scheduler because this is always true

    // start a scheduling ONO ! 
    scheduler_lock.write_acquire();

    asm volatile("cli");
//    Cpu::current()->interrupt_hold();

    // cpu_runned is the NEXT task
    cpu_running[Cpu::currentId()]->sched().block_event = event;

    scheduler_lock.write_release();

    asm volatile("int $101");
    asm volatile("sti");

    // Here we can have a schedule, then we are back onto another CPU 
    // So this cpu has no idea about the interrupt state
    // So Cpu[0]::current()->interrupt_hold(); 
    // = SCHEDULE 
    // Now Cpu[0]::current() is another CPU
    // Cpu[1]::current()->interrupt_release(); errror ! 



    // FIXME: here there is a small world where I have an interrupt between releasing the lock and rescheduling myself

    return {};
} // namespace kernel

core::Result<void> resolve_blocked_tasks()
{
    // Cpu::current()->interrupt_hold();

    scheduler_lock.write_acquire();
    Cpu::current()->interrupt_hold();
    resolve_blocked_tasks_scheduler().assert();
    scheduler_lock.write_release();
    Cpu::current()->interrupt_release();

    return {};
}

} // namespace kernel