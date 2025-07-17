
#pragma once

#include "libcore/lock/lock.hpp"
#include "libcore/result.hpp"
namespace kernel
{

struct CpuContextLaunch
{
    void *entry;
    void *stack_ptr;
    void *kernel_stack_ptr;
    uintptr_t args[5];
    bool user;
};

class CpuContext
{

public:
    void *stack_ptr;
    void *kernel_stack_ptr;

    void *stack_top;
    void *kernel_stack_top;

    core::Lock lock;
    bool await_save;
    bool await_load;

    static core::Result<CpuContext *> create_empty();
    static core::Result<CpuContext *> create(CpuContextLaunch launch);

    void use_stack_addr(uintptr_t addr);

    core::Result<void> prepare(CpuContextLaunch launch);

    void save_in(void volatile *state);
    void load_to(void volatile *state);
    void release();

    template <typename T>
    T *as()
    {
        return static_cast<T *>(this);
    }
    template <typename T>
    T const *as() const
    {
        return static_cast<T const *>(this);
    }
};
} // namespace kernel