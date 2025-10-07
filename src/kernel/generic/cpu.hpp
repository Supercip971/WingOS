
#pragma once

// generally used by implementation
#include "kernel/generic/space.hpp"
#include "kernel/generic/task.hpp"

using CoreId = int;
static constexpr CoreId CpuCoreNone = -1;
class Cpu
{

    // used for syscall handling -- <!> ORDER IS IMPORTANT FOR ASM CODE <!>
public:
    void *syscall_stack;
    uintptr_t saved_stack; // used to save the stack pointer before syscall
    uintptr_t debug_saved_syscall_stackframe;

protected:
    //
    CoreId _id;

    size_t interrupt_depth = 0;
    bool _in_interrupt = false;
    bool _present;

    kernel::Task *_current_task = nullptr;

public:
    bool in_interrupt()
    {
        return _in_interrupt;
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

    kernel::Task *currentTask() const { return _current_task; }
    void currentTask(kernel::Task *task) { _current_task = task; }

    void interrupt_hold();

    void interrupt_release();
};
