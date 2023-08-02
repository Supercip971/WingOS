#pragma once

#include <libcore/io/reader.hpp>
#include <libcore/io/writer.hpp>

namespace core
{

// Always returns 0
// May be used when you don't want to write/read to a stream
class VoidRW : public Reader, public Writer
{
public:
    constexpr VoidRW() {}
    virtual Result<size_t> read(char *buffer, size_t size) const override
    {
        (void)buffer;
        (void)size;
        return size_t(0);
    }

    virtual Result<void> write(const char *data, size_t size) override
    {

        (void)data;
        (void)size;
        return {};
    }

    constexpr ~VoidRW() override
    {
    }
};

} // namespace core