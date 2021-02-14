#include "wstring.h"
#include <string.h>
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
} // namespace utils
