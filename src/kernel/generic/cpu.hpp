
#pragma once

// generally used by implementation
#include "kernel/generic/task.hpp"
class Cpu
{
protected:
    int _id;

    bool _present;

    kernel::Task *_current_task = nullptr;

public:
    int id() const { return _id; };

    bool present() const { return _present; };

    Cpu(int id, bool present) : _id(id), _present(present) {};

    Cpu() : _id(-1), _present(false) {};

    static int currentId();
    static Cpu *current();

    static Cpu *get(int id);

    kernel::Task *currentTask() const { return _current_task; }
    void currentTask(kernel::Task *task) { _current_task = task; }
};
