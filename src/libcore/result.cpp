#include "libcore/result.hpp"

#include "libcore/fmt/log.hpp"

void assert_dump(const char *error)
{
    fmt::log("Result assert failed: {}", error);
}
