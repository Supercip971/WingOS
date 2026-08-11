
#pragma once
#include <kernel/generic/blocker.hpp>
#include <stddef.h>
#include <stdint.h>

#include "iol/lock_context.h"

#include "kernel/generic/cpu.hpp"
#include "libcore/logic.hpp"
#include "libcore/result.hpp"

namespace kernel
{

using TUID = uint64_t;

static constexpr size_t TASK_QUEUE_COUNT = 128;
class Task;

struct SchedulerControlBlock
{
    bool is_idle = false;
    CoreId cpu_affinity = CpuCoreNone;
    CoreId old_cpu_affinity = CpuCoreNone;
    size_t total_cycles = 0;
    BlockMutex mutex = {};

    void block();
    void unblock();

    // note that running and sleeping are not counted in terms of ticks but rather are weighted
    long running = 0;
    long sleeping = 0;
    long priority = 0;

    SchedulerControlBlock() : priority(0) {}

    SchedulerControlBlock(size_t _priority) : priority(_priority) {}

    void tick()
    {
        total_cycles++;
    }

    long weight() const
    {
        return priority + running - sleeping;
    }

    long queue() const
    {
        return fc::clamp(weight() + (long)TASK_QUEUE_COUNT / 2, 0, (long)TASK_QUEUE_COUNT - 1);
    }
};

fc::Result<void> scheduler_init(int cpu_count);

fc::Result<void> scheduler_start();

fc::Result<Task *> next_task_select(CoreId core);

fc::Result<void> scheduler_tick();

fc::Result<void> task_run(TUID task_id, CoreId core = 0);

fc::Result<Task *> schedule(Task *current, void *state, CoreId core, bool soft = false);
fc::Result<void> dump_current_running_task(bool complete);

fc::Result<void> dump_all_current_running_tasks();

fc::Result<void> block_current_task();

// block current and unblock next

bool try_disable_scheduler();

void reenable_scheduler();
fc::Result<void> resolve_blocked_tasks();

} // namespace kernel

// arch implemented
void trigger_reschedule(CoreId cpu);
void trigger_reschedule_unblocked(CoreId cpu);

void reschedule_self();

void scheduler_dump_all();
