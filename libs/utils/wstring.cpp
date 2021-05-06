#include "wstring.h"
#include <string.h>
#include <utils/string_util.h>
namespace utils
{

    template <>
    string to_str<const char *>(const char *value)
    {
        string v(value);
        return v;
    }
    template <>
    string to_str<char *>(char *value)
    {
        string v((const char *)value);
        return v;
    }

    template <typename T>
    string to_str_from_int(T value)
    {

        char *var = new char[32];

        utils::int_to_string<T>(var, 'd', value);

        string v = string(var);
        delete[] var;
        return v;
    }
    template <>
    string to_str<int>(int value)
    {
        return to_str_from_int(value);
    }
    template <>
    string to_str<uint32_t>(uint32_t value)
    {
        return to_str_from_int(value);
    }
    template <>
    string to_str<uint64_t>(uint64_t value)
    {
        return to_str_from_int(value);
    }
    template <>
    string to_str<long>(long value)
    {
        return to_str_from_int(value);
    }
} // namespace utils
