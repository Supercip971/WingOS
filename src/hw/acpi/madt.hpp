
#pragma once

#include <stdint.h>

#include "hw/mem/addr_space.hpp"

#include "hw/acpi/rsdt.hpp"
#include "libcore/fmt/log.hpp"
#include "libcore/result.hpp"
#include "libcore/str.hpp"
#include "libcore/type/trait.hpp"
namespace hw::acpi
{

typedef uint16_t ApicInterruptFlag;

enum ApicInterruptFlagBits
{
    APIC_INTERRUPT_ACTIVE_LOW = 0x2,
    APIC_INTERRUPT_LEVEL_TRIGGERED = 0x8,
};

struct [[gnu::packed]] MadtEntry
{
    uint8_t type;
    uint8_t length;
};

struct [[gnu::packed]] MadtEntryLapic
{
    MadtEntry header;

    constexpr static uint8_t HeaderType = 0;
    constexpr static core::Str name = "LAPIC";

    uint8_t acpi_processor_id;
    uint8_t apic_id;
    uint32_t flags;
};

struct [[gnu::packed]] MadtEntryIoapic
{
    MadtEntry header;

    constexpr static uint8_t HeaderType = 1;
    constexpr static core::Str name = "IOAPIC";

    uint8_t ioapic_id;
    uint8_t _reserved;
    uint32_t ioapic_addr;
    uint32_t global_system_interrupt_base;
};

struct [[gnu::packed]] MadtEntryIso
{
    MadtEntry header;

    constexpr static uint8_t HeaderType = 2;
    constexpr static core::Str name = "ISO";

    uint8_t bus_source;
    uint8_t irq_source;
    uint32_t global_system_interrupt;
    uint16_t flags;
};
struct [[gnu::packed]] MadtEntryIoapicNmi
{
    MadtEntry header;

    constexpr static uint8_t HeaderType = 3;
    constexpr static core::Str name = "IOAPIC NMI";

    uint8_t ioapic_id;
    uint8_t _reserved;
    uint16_t flags;
    uint32_t global_system_interrupt;
};

struct [[gnu::packed]] MadtEntryLapicNmi
{
    MadtEntry header;

    constexpr static uint8_t HeaderType = 4;
    constexpr static core::Str name = "LAPIC NMI";

    uint8_t acpi_processor_id;
    uint16_t flags;
    uint8_t local_apic_lint;
};

struct [[gnu::packed]] MadtEntryLapicOverride
{
    MadtEntry header;

    constexpr static uint8_t HeaderType = 5;
    constexpr static core::Str name = "LAPIC Override";

    uint16_t reserved;
    uint64_t local_apic_addr;
};

struct [[gnu::packed]] MadtEntryLapicX2
{
    MadtEntry header;

    constexpr static uint8_t HeaderType = 9;
    constexpr static core::Str name = "L(x2)APIC";

    uint16_t reserved;
    uint32_t processor_local_x2apicID;
    uint32_t flags;
    uint32_t acpi_id;
};

template <typename T>
concept MadtEntryT = requires(T entry) {
    {
        entry.header.type
    } -> core::IsConvertibleTo<uint8_t>;
    {
        entry.header.length
    } -> core::IsConvertibleTo<uint8_t>;
    {
        T::HeaderType
    } -> core::IsConvertibleTo<uint8_t>;
    {
        T::name
    } -> core::IsConvertibleTo<core::Str>;
};

struct [[gnu::packed]] Madt
{
    SdtHeader header;

    static constexpr core::Str signature = "APIC";

    uint32_t local_apic_addr;
    uint32_t flags;

    uint8_t entries[];

    size_t len() const
    {
        return header.child_len() - sizeof(uint32_t) * 2;
    }

    MadtEntry *begin()
    {
        return reinterpret_cast<MadtEntry *>(entries);
    }

    MadtEntry *end()
    {
        return reinterpret_cast<MadtEntry *>(reinterpret_cast<uint8_t *>(this) + len());
    }

    MadtEntry *next(MadtEntry *entry)
    {
        return reinterpret_cast<MadtEntry *>(reinterpret_cast<uint8_t *>(entry) + entry->length);
    }

    template <MadtEntryT T>
    core::Result<T *> find_entry()
    {
        for (MadtEntry *entry = begin(); entry < end(); entry = next(entry))
        {
            if (entry->type == T::HeaderType)
            {
                return reinterpret_cast<T *>(entry);
            }
        }
        return "Madt: Entry not found";
    }

    template <MadtEntryT T, typename Fn>
    void foreach_entry(Fn fn)
    {
        for (MadtEntry *entry = begin(); entry < end(); entry = next(entry))
        {
            if (entry->type == T::HeaderType)
            {
                fn(reinterpret_cast<T *>(entry));
            }
        }
    }

    void dump()
    {
        int i = 0;
        log::log$("madt: ");
        for (MadtEntry *entry = begin(); entry < end(); entry = next(entry))
        {
            log::log$("- entry[{}]: {}", i, entry->type);

            i++;
        }
    }
};
}; // namespace hw::acpi
