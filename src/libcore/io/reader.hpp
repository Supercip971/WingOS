#pragma once

#include <libcore/io/seekable.hpp>
#include <libcore/result.hpp>
#include <stddef.h>

namespace core
{

class Reader
{
public:
    virtual Result<size_t> read(char *data, size_t size) const
    {
        (void)data;
        (void)size;
        return size_t(0);
    };

    constexpr virtual ~Reader()
    {
    }
};

template <typename T>
concept Readable = requires(T *x) {
    {
        x->read((char *)0, 10)
    } -> IsConvertibleToResult<size_t>;
};

static_assert(Readable<Reader>);

} // namespace core