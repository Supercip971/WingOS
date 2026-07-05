

#include <libcore/io/void.hpp>
#include <libcore/io/writer.hpp>

#include "libcore/fmt/log.hpp"
#include "libcore/lock/lock.hpp"

namespace fmt
{

fc::Lock _log_lock = {};
static fc::VoidRW default_target{};
static fc::Writer *target = &default_target;

void log_lock()
{
    _log_lock.lock();
}

void log_release()
{
    _log_lock.force_unlock();
}

void provide_log_target(fc::Writer *writer)
{
    target = writer;
}

fc::Writer *log_target()
{
    return target;
}

} // namespace fmt

void fc::debug_provide_info(const char *info, const char *data)
{
    fmt::log$("{} {}", info, data);
}
