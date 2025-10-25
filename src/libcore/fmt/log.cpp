

#include <libcore/io/void.hpp>
#include <libcore/io/writer.hpp>
#include <libcore/str.hpp>

#include "libcore/fmt/log.hpp"


#include "libcore/lock/lock.hpp"

namespace log
{


core::Lock _log_lock;
static core::VoidRW default_target{};
static core::Writer *target = &default_target;


void log_lock()
{
    _log_lock.lock();
}

void log_release()
{
    _log_lock.release();
}
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