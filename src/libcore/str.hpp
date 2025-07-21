#pragma once
#include <libcore/io/reader.hpp>
#include <libcore/mem/view.hpp>
#include <libcore/type-utils.hpp>
#include <stddef.h>

namespace core
{
class Str : public MemView<char>
{

    static constexpr size_t compute_len(const char *str)
    {
        size_t i = 0;
        while (str[i] != '\0')
        {
            i++;
        }
        return i;
    }

public:
    constexpr Str(const char *str) : MemView(str, Str::compute_len(str)) {};
    constexpr Str(const char *str, int len) : MemView(str, len) {};

    constexpr Str() : MemView("", 0) {};

    constexpr Str(Str &&other)
        : MemView(other._data, other._len)
    {
    }

    constexpr Str(const Str &str)
        : MemView(str._data, str._len) {}

    constexpr Str &operator=(const Str &str)
    {
        _data = str._data;
        _len = str._len;
        return *this;
    }

    constexpr Str &operator=(const char *str)
    {
        _data = str;
        _len = compute_len(str);
        return *this;
    }

    constexpr operator bool() const
    {
        return _len > 0 && _data != nullptr;
    }

    const MemView<char> &view() const
    {
        return *this;
    }

    constexpr Str substr(int start, int end) const
    {
        if (end <= start)
        {
            return Str();
        }
        return Str(_data + start, end - start);
    }

    constexpr Str substr(int start) const
    {
        return substr(start, _len);
    }

    constexpr Str sub_last_char(char v) const
    {
        int last = _len + 1;

        for (size_t i = 0; i < _len; i++)
        {
            if (_data[i] == v)
            {
                last = i;
            }
        }

        return substr(last);
    }

    constexpr bool start_with(const Str &str) const
    {
        if (str._len > _len)
        {
            return false;
        }
        for (size_t i = 0; i < str._len; i++)
        {
            if (_data[i] != str._data[i])
            {
                return false;
            }
        }
        return true;
    }

    
};

constexpr char *StrChr(char *str, char c)
{
    char *last = nullptr;
    while (*str != '\0')
    {
        if (*str == c)
        {
            last = str;
        }
        str++;
    }
    return last;
}

} // namespace core