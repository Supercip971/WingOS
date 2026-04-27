
#pragma once

// generally used by implementation
#include <libcore/lock/lock.hpp>
#include <stddef.h>
#include <stdint.h>
#include <libcore/fmt/log.hpp>
using CoreId = int;
static constexpr CoreId CpuCoreNone = -1;

struct DebuggedContext
{
    const char *last_syscall;
    size_t last_syscall_id;
    size_t last_syscall_task_called;
    bool in_syscall;

    void dump() const {
        log::log$("DebuggedContext:");
        log::log$("  - last_syscall_id: {}", last_syscall_id);
        log::log$("  - last_syscall_task_called: {}", last_syscall_task_called);
        log::log$("  - in_syscall: {}", in_syscall ? "true" : "false");
    }
};

 enum CpuSchedStates : int {
    CPU_SCHED_RUNNING,
    CPU_SCHED_SWAP_SINGLE,
    CPU_SCHED_SWAP_GROUP,
    CPU_SCHED_DIRECTING,
};

static inline const char* cpuSchedStateToStr(CpuSchedStates state) {
    switch(state) {
        case CPU_SCHED_RUNNING: return "RUNNING";
        case CPU_SCHED_SWAP_SINGLE: return "SWAP_SINGLE";
        case CPU_SCHED_SWAP_GROUP: return "SWAP_GROUP";
        case CPU_SCHED_DIRECTING: return "DIRECTING";
        default: return "UNKNOWN";
    }
}
namespace kernel
{
class Task;
};
class [[gnu::packed]] Cpu
{

    // used for syscall handling -- <!> ORDER IS IMPORTANT FOR ASM CODE <!>
public:
    uintptr_t syscall_stack;

    uintptr_t debug_saved_syscall_stackframe;

protected:
    //
    CoreId _id;

    size_t interrupt_depth = 0;
    bool _in_interrupt = false;
    bool _present;
    bool _in_syscall_lock = false;


public:

    kernel::Task * _current_task = nullptr;
    DebuggedContext __attribute__((aligned(16))) debug_context;

    CpuSchedStates sched_state = CpuSchedStates::CPU_SCHED_RUNNING;
    bool in_interrupt()
    {
        return _in_interrupt;
    }

    void set_current_task(kernel::Task *task)
    {
        _current_task = task;
    }


    // apply after every syscall information required are retrieved
    static bool begin_syscall();
    static bool end_syscall();

    bool has_syscall_lock() const
    {
        return _in_syscall_lock;
    }

    void in_interrupt(bool val)
    {
        _in_interrupt = val;
    }

    int id() const { return _id; };

    bool present() const { return _present; };

    Cpu(int id, bool present) : _id(id), _present(present) {};

    Cpu() : _id(-1), _present(false) {};

    static CoreId currentId();
    static Cpu *current();

    static Cpu *get(int id);

    static size_t count();

    kernel::Task *currentTask() const;

    void interrupt_hold();

    void interrupt_release(bool re_enable_int = true);
};
