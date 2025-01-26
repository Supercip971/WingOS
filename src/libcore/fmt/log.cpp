

#include <libcore/io/void.hpp>
#include <libcore/io/writer.hpp>
#include <libcore/str.hpp>

#include "libcore/fmt/log.hpp"
namespace log
{

static core::VoidRW default_target{};
static core::Writer *target = &default_target;
void provide_log_target(core::Writer *writer)
{
    target = writer;
}

core::Writer *log_target()
{
    return target;
}

} // namespace log
void core::debug_provide_info(const char *info, const char *data)
{
    log::log$("{} {}", info, data);
}