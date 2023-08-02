

#include <libcore/io/void.hpp>
#include <libcore/io/writer.hpp>
#include <libcore/str.hpp>
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