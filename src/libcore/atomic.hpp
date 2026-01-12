#pragma once

#ifdef __ck_host__
// For host builds, use custom atomic implementation to avoid system header conflicts
namespace core
{

enum class MemoryOrder
{
    Relaxed = __ATOMIC_RELAXED,
    Consume = __ATOMIC_CONSUME,
    Acquire = __ATOMIC_ACQUIRE,
    Release = __ATOMIC_RELEASE,
    AcqRel = __ATOMIC_ACQ_REL,
    SeqCst = __ATOMIC_SEQ_CST
};

template <typename T>
class Atomic
{
    T _value;

public:
    Atomic() : _value(T{}) {}
    Atomic(T value) : _value(value) {}

    // Delete copy and move
    Atomic(const Atomic &) = delete;
    Atomic &operator=(const Atomic &) = delete;
    Atomic(Atomic &&) = delete;
    Atomic &operator=(Atomic &&) = delete;

    T load(MemoryOrder order = MemoryOrder::SeqCst) const
    {
        return __atomic_load_n(&_value, static_cast<int>(order));
    }

    void store(T value, MemoryOrder order = MemoryOrder::SeqCst)
    {
        __atomic_store_n(&_value, value, static_cast<int>(order));
    }

    T exchange(T value, MemoryOrder order = MemoryOrder::SeqCst)
    {
        return __atomic_exchange_n(&_value, value, static_cast<int>(order));
    }

    bool compare_exchange_strong(T &expected, T desired,
                                  MemoryOrder success = MemoryOrder::SeqCst,
                                  MemoryOrder failure = MemoryOrder::SeqCst)
    {
        return __atomic_compare_exchange_n(&_value, &expected, desired, false,
                                           static_cast<int>(success),
                                           static_cast<int>(failure));
    }

    bool compare_exchange_weak(T &expected, T desired,
                                MemoryOrder success = MemoryOrder::SeqCst,
                                MemoryOrder failure = MemoryOrder::SeqCst)
    {
        return __atomic_compare_exchange_n(&_value, &expected, desired, true,
                                           static_cast<int>(success),
                                           static_cast<int>(failure));
    }

    T fetch_add(T arg, MemoryOrder order = MemoryOrder::SeqCst)
    {
        return __atomic_fetch_add(&_value, arg, static_cast<int>(order));
    }

    T fetch_sub(T arg, MemoryOrder order = MemoryOrder::SeqCst)
    {
        return __atomic_fetch_sub(&_value, arg, static_cast<int>(order));
    }

    T fetch_and(T arg, MemoryOrder order = MemoryOrder::SeqCst)
    {
        return __atomic_fetch_and(&_value, arg, static_cast<int>(order));
    }

    T fetch_or(T arg, MemoryOrder order = MemoryOrder::SeqCst)
    {
        return __atomic_fetch_or(&_value, arg, static_cast<int>(order));
    }

    T fetch_xor(T arg, MemoryOrder order = MemoryOrder::SeqCst)
    {
        return __atomic_fetch_xor(&_value, arg, static_cast<int>(order));
    }

    // Operators
    T operator++()
    {
        return fetch_add(1) + 1;
    }

    T operator++(int)
    {
        return fetch_add(1);
    }

    T operator--()
    {
        return fetch_sub(1) - 1;
    }

    T operator--(int)
    {
        return fetch_sub(1);
    }

    T operator+=(T arg)
    {
        return fetch_add(arg) + arg;
    }

    T operator-=(T arg)
    {
        return fetch_sub(arg) - arg;
    }

    T operator&=(T arg)
    {
        return fetch_and(arg) & arg;
    }

    T operator|=(T arg)
    {
        return fetch_or(arg) | arg;
    }

    T operator^=(T arg)
    {
        return fetch_xor(arg) ^ arg;
    }

    operator T() const
    {
        return load();
    }
};

static inline void atomic_cache_flush()
{
    // This is a no-op on x86_64, as the cache is flushed automatically
    // on context switches and memory accesses.
    // However, we can use this function to ensure that the compiler does not optimize away
    // any atomic operations that we perform.
    asm volatile("mfence" ::: "memory");
}

static inline void atomic_cache_sync()
{
    // This is a no-op on x86_64, as the cache is flushed automatically
    // on context switches and memory accesses.
    // However, we can use this function to ensure that the compiler does not optimize away
    // any atomic operations that we perform.
    asm volatile("sfence" ::: "memory");
}

} // namespace core

#else
// For kernel and embedded targets, use std::atomic from the standard library
#include <atomic>

namespace core
{

enum class MemoryOrder
{
    Relaxed = static_cast<int>(std::memory_order_relaxed),
    Consume = static_cast<int>(std::memory_order_consume),
    Acquire = static_cast<int>(std::memory_order_acquire),
    Release = static_cast<int>(std::memory_order_release),
    AcqRel = static_cast<int>(std::memory_order_acq_rel),
    SeqCst = static_cast<int>(std::memory_order_seq_cst)
};

// Wrapper around std::atomic that accepts core::MemoryOrder
template <typename T>
class Atomic
{
    std::atomic<T> _value;

    static constexpr std::memory_order to_std_order(MemoryOrder order)
    {
        return static_cast<std::memory_order>(static_cast<int>(order));
    }

public:
    Atomic() : _value(T{}) {}
    Atomic(T value) : _value(value) {}

    // Delete copy and move
    Atomic(const Atomic &) = delete;
    Atomic &operator=(const Atomic &) = delete;
    Atomic(Atomic &&) = delete;
    Atomic &operator=(Atomic &&) = delete;

    T load(MemoryOrder order = MemoryOrder::SeqCst) const
    {
        return _value.load(to_std_order(order));
    }

    void store(T value, MemoryOrder order = MemoryOrder::SeqCst)
    {
        _value.store(value, to_std_order(order));
    }

    T exchange(T value, MemoryOrder order = MemoryOrder::SeqCst)
    {
        return _value.exchange(value, to_std_order(order));
    }

    bool compare_exchange_strong(T &expected, T desired,
                                  MemoryOrder success = MemoryOrder::SeqCst,
                                  MemoryOrder failure = MemoryOrder::SeqCst)
    {
        return _value.compare_exchange_strong(expected, desired, 
                                              to_std_order(success),
                                              to_std_order(failure));
    }

    bool compare_exchange_weak(T &expected, T desired,
                                MemoryOrder success = MemoryOrder::SeqCst,
                                MemoryOrder failure = MemoryOrder::SeqCst)
    {
        return _value.compare_exchange_weak(expected, desired,
                                            to_std_order(success),
                                            to_std_order(failure));
    }

    T fetch_add(T arg, MemoryOrder order = MemoryOrder::SeqCst)
    {
        return _value.fetch_add(arg, to_std_order(order));
    }

    T fetch_sub(T arg, MemoryOrder order = MemoryOrder::SeqCst)
    {
        return _value.fetch_sub(arg, to_std_order(order));
    }

    T fetch_and(T arg, MemoryOrder order = MemoryOrder::SeqCst)
    {
        return _value.fetch_and(arg, to_std_order(order));
    }

    T fetch_or(T arg, MemoryOrder order = MemoryOrder::SeqCst)
    {
        return _value.fetch_or(arg, to_std_order(order));
    }

    T fetch_xor(T arg, MemoryOrder order = MemoryOrder::SeqCst)
    {
        return _value.fetch_xor(arg, to_std_order(order));
    }

    operator T() const
    {
        return load();
    }
};

static inline void atomic_cache_flush()
{
    std::atomic_thread_fence(std::memory_order_seq_cst);
}

static inline void atomic_cache_sync()
{
    std::atomic_thread_fence(std::memory_order_release);
}

} // namespace core

#endif