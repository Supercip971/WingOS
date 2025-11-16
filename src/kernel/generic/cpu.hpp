
#pragma once

// generally used by implementation
#include <stdint.h>
#include <stddef.h>
#include <libcore/lock/lock.hpp>
using CoreId = int;
static constexpr CoreId CpuCoreNone = -1;



namespace kernel {
    class Task;
};
class [[gnu::packed]] Cpu 
{

    // used for syscall handling -- <!> ORDER IS IMPORTANT FOR ASM CODE <!>
public:
    void * volatile syscall_stack;
    volatile uintptr_t saved_stack; // used to save the stack pointer before syscall
    uintptr_t debug_saved_syscall_stackframe;

protected:
    //
    CoreId _id;

    size_t interrupt_depth = 0;
    bool _in_interrupt = false;
    bool _present;

public:

    bool in_interrupt()
    {
        return _in_interrupt;
    }

    // apply after every syscall information required are retrieved
    static bool enter_syscall_safe_mode();
    static bool exit_syscall_safe_mode();
    static bool end_syscall();

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
