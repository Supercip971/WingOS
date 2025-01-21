#pragma once

#include <stdlib.h>

#include "libcore/result.hpp"
#include "libcore/type-utils.hpp"
#include "libcore/type/trait.hpp"

// unstable memory is set when the memory state
// has been known to contain error, so that
// the log system may be able to log the error without doing
// any further allocation
static bool unstable_memory = false;

namespace core
{

static inline Result<void *> mem_alloc(size_t bytes)
{
    void *v = malloc(bytes);

    if (v == nullptr)
    {
        return Result<void *>::error("unable to allocate memory");
    }

    return v;
}

template <typename T>
static inline Result<T *> mem_alloc(size_t count)
{
    return static_cast<T *>(try$(mem_alloc(sizeof(T) * count)));
}

template <typename T>
static inline Result<T *> mem_alloc()
{
    return static_cast<T *>(try$(mem_alloc(sizeof(T))));
}

static inline void mem_free(void *addr)
{
    free(addr);
}

static inline Result<void *> mem_realloc(void *old_ptr, size_t bytes)
{
    void *res = realloc(old_ptr, bytes);

    if (res == nullptr)
    {
        return Result<void *>::error("unable to reallocate memory");
    }

    return res;
}

template <typename T>
static inline Result<T *> mem_realloc(void *old_ptr, size_t count)
{
    return static_cast<T *>(try$(mem_realloc(old_ptr, count * sizeof(T))));
}

class DefaultAllocator : public NoCopy
{
    [[maybe_unused]] void *state;

public:
    DefaultAllocator() {};

    DefaultAllocator(DefaultAllocator &&) = default;
    DefaultAllocator &operator=(DefaultAllocator &&) = default;

    Result<void *> allocate(size_t size)
    {

        void *res = malloc(size);

        if (res == nullptr) [[unlikely]]
        {
            unstable_memory = true;
            return Result<void *>::error("Failed to allocate memory");
        }
        return res;
    }

    template <typename T>
    Result<T *> allocate(int count)
    {
        return static_cast<T *>(try$(allocate(count * sizeof(T))));
    }

    void release(void *ptr)
    {
        free(ptr);
    }

    Result<void *> reallocate(void *ptr, size_t size)
    {
        void *res = realloc(ptr, size);

        if (res == nullptr) [[unlikely]]
        {
            unstable_memory = true;
            return Result<void *>::error("Failed to reallocate memory");
        }
        return res;
    }

    template <typename T>
    Result<T *> reallocate(T *ptr, size_t count)
    {
        return static_cast<T *>(try$(reallocate(ptr, count * sizeof(T))));
    }
};

template <typename T>
concept MemAllocator = requires(T a) {
    {
        a.allocate(16)
    } -> IsConvertibleTo<Result<void *>>;
    {
        a.release(nullptr)
    } -> IsConvertibleTo<void>;
    {
        a.reallocate(nullptr, 16)
    } -> IsConvertibleTo<Result<void *>>;
};

static_assert(MemAllocator<DefaultAllocator>);

} // namespace core