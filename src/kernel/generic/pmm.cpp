#include "pmm.hpp"
#include <libcore/fmt/log.hpp>
static Pmm instance;
Pmm &Pmm::the()
{
    return instance;
}
core::Result<void> Pmm::initialize(const mcx::MachineContext *context)
{
    instance = try$(Pmm::create(context));
    return {};
}