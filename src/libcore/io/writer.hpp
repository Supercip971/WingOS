#pragma once

#include <libcore/io/reader.hpp>
#include <libcore/io/seekable.hpp>
#include <libcore/mem/view.hpp>
#include <stddef.h>
#include <stdint.h>
namespace core
{

class Writer
{
public:
    constexpr virtual Result<void> write(const char *data, size_t size)
    {
        (void)data;
        (void)size;
        return {};
    };

    template <Viewable T>
    constexpr Result<void> write(T view)
        requires(Viewable<T>)
    {
        return write(view.data(), view.len());
    }
    template <Viewable T>
    constexpr Result<void> writeV(T view)
        requires(Viewable<T>)
    {
        return write(view.data(), view.len());
    }

    constexpr virtual ~Writer()
    {
    }
};

template <typename T>
concept Writable = requires(T *x) {
    {
        x->write((const char *)0, 10)
    } -> IsConvertibleToResult<void>;
    {
        x->write(MemView<char>())
    } -> IsConvertibleToResult<void>;
};


static_assert(Writable<Writer>);

} // namespace core