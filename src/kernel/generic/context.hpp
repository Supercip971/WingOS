
#pragma once

#include "kernel/generic/paging.hpp"
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
    volatile bool await_save;
    volatile bool await_load;

    VmmSpace *_vmm_space;
    static core::Result<CpuContext *> create_empty();
    static core::Result<CpuContext *> create(CpuContextLaunch launch);

    void use_stack_addr(uintptr_t addr);

    core::Result<void> prepare(CpuContextLaunch launch);
    void dump();

    void save_in(void *state);
    void load_to(void *state);
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