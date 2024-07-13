
#include "hw/acpi/madt.hpp"
#include "hw/acpi/rsdp.hpp"
#include "libcore/result.hpp"
#include "mcx/mcx.hpp"
namespace hw::acpi
{

using CpuDetectedFunc = core::Result<void>(MadtEntryLapic const *);

size_t apic_cpu_count();

core::Result<void> apic_initialize(mcx::MachineContext const *context, CpuDetectedFunc *cpu_callback);
} // namespace hw::acpi
