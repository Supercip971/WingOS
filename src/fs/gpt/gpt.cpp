#include "gpt.hpp"
#include <libcore/fmt/log.hpp>
#include <libcore/result.hpp>
#include <protocols/disk/disk.hpp>

#include "libcore/str_writer.hpp"

#include "iol/wingos/asset.hpp"
#include "libcore/type-utils.hpp"
#include "math/align.hpp"
#include "wingos-headers/asset.h"

fc::Result<Wingos::GPTDiskParseResult> Wingos::parse_gpt(fc::Str &device)
{
    Wingos::GPTDiskParseResult result = {};

    auto connection = try$(prot::DiskConnection::connect(device));

    Wingos::MemoryAsset header_asset = (Wingos::Space::self().allocate_physical_memory(4096));

    try$(connection.read(header_asset, 1, 512));
    Wingos::VirtualMemoryAsset header_mapping = Wingos::Space::self().map_memory(header_asset, ASSET_MAPPING_FLAG_READ | ASSET_MAPPING_FLAG_WRITE);

    GPT *gpt_header = (GPT *)header_mapping.ptr();
    // dump:
    fmt::log$("GPT Signature: {}", fc::Str(gpt_header->signature, 8));
    fmt::log$("GPT Revision: {}", fc::copy(gpt_header->revision));
    fmt::log$("GPT Size: {}", fc::copy(gpt_header->size));
    fmt::log$("GPT Partition Entry LBA: {}", fc::copy(gpt_header->lba_start_guid_partition_entry));
    fmt::log$("GPT Partition Count: {}", fc::copy(gpt_header->partition_count));
    fmt::log$("GPT Partition Entry Size: {}", fc::copy(gpt_header->partition_entry_size));

    result.header = *gpt_header;

    // parse partition entries
    size_t partition_entries_size = gpt_header->partition_count * gpt_header->partition_entry_size;
    size_t partition_entries_sectors = math::alignUp((partition_entries_size), (size_t)512);
    Wingos::MemoryAsset partition_entries_asset = (Wingos::Space::self().allocate_physical_memory(math::alignUp(partition_entries_sectors, (size_t)4096)));
    try$(connection.read(partition_entries_asset, gpt_header->lba_start_guid_partition_entry, partition_entries_sectors));
    Wingos::VirtualMemoryAsset partition_entries_mapping = Wingos::Space::self().map_memory(partition_entries_asset, ASSET_MAPPING_FLAG_READ | ASSET_MAPPING_FLAG_WRITE);

    GPTPartitionEntries *partition_entries = (GPTPartitionEntries *)partition_entries_mapping.ptr();

    for (size_t i = 0; i < gpt_header->partition_count; i++)
    {
        GPTPartitionEntries *entry = (GPTPartitionEntries *)((uint8_t *)partition_entries + i * gpt_header->partition_entry_size);
        // check if partition type GUID is not zero
        bool is_empty = true;
        for (size_t j = 0; j < 16; j++)
        {
            if (entry->part_type_guid[j] != 0)
            {
                is_empty = false;
                break;
            }
        }
        if (is_empty)
        {
            continue;
        }

        // get partition name
        fc::WStr part_name;
        for (size_t j = 0; j < 36; j++)
        {
            if (entry->name[j] == 0)
            {
                break;
            }
            part_name.append(fc::Str((char *)&(entry->name[j]), 1)); // convert char16_t to char
        }
        fmt::log$("Found partition: {} (LBA {} - {})", part_name.view(), fc::copy(entry->lba_start), fc::copy(entry->lba_end));

        GPTDiskParseEntry parse_entry = {};
        parse_entry.name = fc::move(part_name);
        parse_entry.entry = entry;
        result.entries.push(fc::move(parse_entry));
    }

    return (result);
}
