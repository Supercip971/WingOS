
#pragma once

// generally used by implementation
#include "kernel/generic/task.hpp"

using CoreId = int;
static constexpr CoreId CpuCoreNone = -1;
class Cpu
{
protected:
    CoreId _id;

    bool _present;

    kernel::Task *_current_task = nullptr;

public:
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
};
