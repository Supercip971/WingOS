
#include "hw/acpi/lapic.hpp"
#include "hw/acpi/madt.hpp"
#include "hw/acpi/rsdp.hpp"
#include "libcore/result.hpp"
#include "mcx/mcx.hpp"

namespace hw::acpi
{

using CpuDetectedFunc = fc::Result<void>(MadtEntryLapic const *);

size_t apic_cpu_count();

fc::Result<void> apic_initialize(mcx::MachineContext const *context, CpuDetectedFunc *cpu_callback);

fc::Result<void> redirect_interrupt(LCpuId cpu, uint8_t irq, uint8_t vector, bool enabled = true);

fc::Result<void> update_interrupt_source_override(MadtEntryIso const *entry);

hw::acpi::Rsdp *rsdp();

} // namespace hw::acpi
