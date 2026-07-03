#pragma once

#include <libcore/result.hpp>
#include <stddef.h>
#include <stdint.h>

namespace core
{

enum class SeekFrom
{
    FROM_BEGIN,
    FROM_CURRENT,
    FROM_END
};

class Seeker
{
public:
    Seeker() = default;
    virtual ~Seeker();

    constexpr virtual Result<void> seek(size_t offset, SeekFrom from = SeekFrom::FROM_BEGIN)
    {
        (void)offset;
        (void)from;
        return {};
    };

    constexpr virtual Result<size_t> tell() { return {}; };

    constexpr virtual Result<size_t> size()
    {
        size_t current = try$(tell());
        seek(0, SeekFrom::FROM_END);
        size_t size = try$(tell());
        seek(current, SeekFrom::FROM_BEGIN);
        return size;
    }

    constexpr virtual Result<size_t> rewind()
    {
        seek(0, SeekFrom::FROM_BEGIN);
        return tell();
    }
};

template <typename T>
concept Seekable = requires(T *x) {
    {
        x->seek(0, SeekFrom::FROM_BEGIN)
    } -> IsConvertibleToResult<void>;
    {
        x->tell()
    } -> IsConvertibleToResult<size_t>;
    {
        x->size()
    } -> IsConvertibleToResult<size_t>;
    {
        x->rewind()
    } -> IsConvertibleToResult<size_t>;
};

static_assert(Seekable<Seeker>);

} // namespace core
