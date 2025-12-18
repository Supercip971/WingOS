
#pragma once

#include <hw/mem/addr_space.hpp>

#include "hw/acpi/rsdp.hpp"
#include <string.h>
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

struct [[gnu::packed]] SRAT
{
    SdtHeader header;
    uint8_t reserved[8];

    uint8_t data[];
    static constexpr core::Str signature = "SRAT";
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


    for (size_t i = 0; i < (table->header.child_len()) / sizeof(table->entries[0]); i++)
    {
        auto entry = table->entries[i];
        auto *entry_header = toVirt(entry).template as<SdtHeader>();
        auto s1 = core::Str(entry_header->signature, 4);

        if (s1 == K::signature)
            return reinterpret_cast<K *>(entry_header);
    }

    return core::Result<K*>::error("Not found");
}

template <SdtEntry K>
core::Result<K *> rsdt_find(hw::acpi::Rsdp *_rsdp)
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

template<MappCallbackFn T> 
core::Result<void> prepare_mapping(uintptr_t rsdp_addr, T fn)
{
    try$(fn(rsdp_addr, sizeof(hw::acpi::Rsdp)));
    hw::acpi::Rsdp *rsdp = toVirt(rsdp_addr).as<hw::acpi::Rsdp>();
    auto addr = rsdp->rsdt_phys_addr();
    if (addr.type == hw::acpi::RsdtTypes::RSDT)
    {
        auto rsdt = toVirt(addr.physical_addr).as<hw::acpi::Rsdt>();
        try$(fn((uintptr_t)addr.physical_addr, 4096));
        if(rsdt->header.length > 4096)
        {
            try$(fn((uintptr_t)addr.physical_addr, rsdt->header.length));
        }


        for(size_t i = 0; i < (rsdt->header.child_len()) / sizeof(rsdt->entries[0]); i++)
        {
            auto entry = rsdt->entries[i];
            try$(fn((uintptr_t)entry, 4096));
            auto *entry_header = toVirt(entry).template as<SdtHeader>();
            if(entry_header->length > 4096)
            {
                try$(fn((uintptr_t)entry, entry_header->length));
            }
        }
    }
    else // XSDT
    {
        auto xsdt = toVirt(addr.physical_addr).as<hw::acpi::Xsdt>();
        try$(fn((uintptr_t)addr.physical_addr, 4096));
        if(xsdt->header.length > 4096)
        {
            try$(fn((uintptr_t)addr.physical_addr, xsdt->header.length));
        }

        for(size_t i = 0; i < (xsdt->header.child_len()) / sizeof(xsdt->entries[0]); i++)
        {
            auto entry = xsdt->entries[i];
            try$(fn((uintptr_t)entry, 4096));
            auto *entry_header = toVirt(entry).template as<SdtHeader>();
            if(entry_header->length > 4096)
            {
                try$(fn((uintptr_t)entry, entry_header->length));
            }
        }
    }

    return {};


}


template <SdTable T, typename Fn>
void SdtForeach(T *table, Fn callback)
{
    // Avoid address-of-packed-member warning by copying to local array
    // Use a fixed max count for entries to avoid incomplete type error
    constexpr size_t max_entries = 256;
    alignas(alignof(typename T::childType)) typename T::childType entries_copy[max_entries];
    memcpy(entries_copy, table->entries, sizeof(entries_copy));
    typename T::childType *entries = entries_copy;

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
                   auto length = entry->length;
                   log::log$("- length: {}", length);
                   log::log$("- revision: {}", entry->revision);
                   log::log$("- oem_id: {}", s2);
                   log::log$("- oem_table_id: {}", s3); });
}
} // namespace hw::acpi