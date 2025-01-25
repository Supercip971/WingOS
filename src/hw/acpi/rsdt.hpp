
#pragma once

#include <hw/mem/addr_space.hpp>

#include "hw/acpi/rsdp.hpp"
#include "libcore/fmt/log.hpp"
#include "libcore/result.hpp"
#include "libcore/str.hpp"
#include "libcore/type/trait.hpp"
namespace hw::acpi
{


struct [[gnu::packed]] SdtHeader
{
    char signature[4];
    uint32_t length;
    uint8_t revision;
    uint8_t checksum;
    char oem_id[6];
    char oem_table_id[8];
    uint32_t oem_revision;
    uint32_t creator_id;
    uint32_t creator_revision;

    size_t child_len() const
    {
        return length - sizeof(SdtHeader);
    }
};

struct [[gnu::packed]] Rsdt
{
    SdtHeader header;
    uint32_t entries[];

    static constexpr core::Str signature = "RSDT";

    using childType = uint32_t;
};

struct [[gnu::packed]] Xsdt
{
    SdtHeader header;
    PhysAddr entries[];

    static constexpr core::Str signature = "XSDT";

    using childType = PhysAddr;
};

// Maybe I'm over-engineering this
// It's an excuse to explore further more concepts
// And avoiding to use virtual functions
// So that I can easily handle XSDT and RSDT at the same time
template <typename T>
concept SdtEntry = requires(T sdt) {
    {
        sdt.signature
    } -> core::IsConvertibleTo<core::Str>;
    {
        sdt.header
    } -> core::IsConvertibleTo<SdtHeader>;
};

template <typename T>
concept SdTable = SdtEntry<T> &&
                  core::IsConvertibleTo<uintptr_t, typename T::childType>;

static_assert(SdTable<Xsdt>);
static_assert(SdTable<Rsdt>);




template <SdTable T, SdtEntry K>
core::Result<K *> SdtFind(T *table)
{

    typename T::childType *entries = table->entries;

    for (size_t i = 0; i < (table->header.child_len()) / sizeof(entries[0]); i++)
    {
        auto entry = entries[i];
        auto *entry_header = toVirt(entry).template as<SdtHeader>();
        auto s1 = core::Str(entry_header->signature, 4);

        if (s1 == K::signature)
            return reinterpret_cast<K *>(entry_header);
    }

    return "Not found";
}


template <SdtEntry K> 
core::Result<K *> rsdt_find(hw::acpi::Rsdp* _rsdp)
{
    auto addr = _rsdp->rsdt_phys_addr();
    if (addr.type == hw::acpi::RsdtTypes::RSDT)
    {
        auto rsdt = toVirt(addr.physical_addr).as<hw::acpi::Rsdt>();
        return try$((hw::acpi::SdtFind<hw::acpi::Rsdt, K>(rsdt)));
    }
    else // XSDT
    {
        auto xsdt = toVirt(addr.physical_addr).as<hw::acpi::Xsdt>();
        return try$((hw::acpi::SdtFind<hw::acpi::Xsdt, K>(xsdt)));
    }

}

template <SdTable T, typename Fn>
void SdtForeach(T *table, Fn callback)
{
    typename T::childType *entries = table->entries;

    for (size_t i = 0; i < (table->header.child_len()) / sizeof(entries[0]); i++)
    {
        auto entry = entries[i];
        auto *entry_header = toVirt(entry).template as<SdtHeader>();

        callback(entry_header);
    }
}

template <SdTable T>
constexpr void dump(T *d)
{
    SdtForeach(d, [](SdtHeader *entry)
               {
                   auto s1 = core::Str(entry->signature, 4);
                   auto s2 = core::Str(entry->oem_id, 6);
                   auto s3 = core::Str(entry->oem_table_id, 8);

                   log::log$("entry:");
                   log::log$("- signature: {}", s1);
                   log::log$("- length: {}", entry->length);
                   log::log$("- revision: {}", entry->revision);
                   log::log$("- oem_id: {}", s2);
                   log::log$("- oem_table_id: {}", s3); });
}
} // namespace hw::acpi