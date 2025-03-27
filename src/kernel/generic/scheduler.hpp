
#pragma once 
#include "kernel/generic/cpu.hpp"
#include "kernel/generic/task.hpp"
#include "libcore/logic.hpp"
#include "libcore/result.hpp"

namespace kernel 
{


    struct SchedulerEntity {
        bool is_idle = false;
        Task* task = nullptr;
        CoreId cpu_affinity = CpuCoreNone;
        CoreId old_cpu_affinity = CpuCoreNone;
        size_t total_cycles = 0;

        // note that running and sleeping are not counted in terms of ticks but rather are weighted
        long running = 0;
        long sleeping = 0;
        
        long priority = 0;

        SchedulerEntity() : task(nullptr), priority(0) {}
        SchedulerEntity(Task* task, size_t priority) : task(task), priority(priority) {}
        
        void tick() {
            total_cycles++;
        }

        long weight() const {
            return priority + running - sleeping;
        }

        long queue() const {
            return core::clamp(weight() + (long)TASK_QUEUE_COUNT/2, 0, (long)TASK_QUEUE_COUNT-1);
        }
    };

    core::Result<void> scheduler_init(int cpu_count);

    core::Result<void> scheduler_start();

    core::Result<Task*> next_task_select(CoreId core);

    core::Result<void> scheduler_tick();

    core::Result<void> task_run(TUID task_id, CoreId core=0);

    core::Result<Task*> schedule(Task* current, void* state,CoreId core);
}