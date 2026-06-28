#include <libcore/fmt/log.hpp>

extern "C" int stbi__err(const char *str)
{
    fmt::err$("stb error: {}", str);
    return 0;
}
