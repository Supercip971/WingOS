#include "gpt.hpp"
#include <libcore/result.hpp>
#include <protocols/disk/disk.hpp>
#include <libcore/fmt/log.hpp>

core::Result<Wingos::GPTDiskParseResult> Wingos::parse_gpt(core::Str& device)
{
    Wingos::GPTDiskParseResult result = {};

    auto connection = try$(prot::DiskConnection::connect(device));

    Wingos::MemoryAsset header_asset = (Wingos::Space::self().allocate_physical_memory(4096));

    
    auto read_res = connection.read(header_asset, 1, 512);

    Wingos::VirtualMemoryAsset header_mapping = Wingos::Space::self().map_memory(header_asset, ASSET_MAPPING_FLAG_READ | ASSET_MAPPING_FLAG_WRITE);

    
    GPT* gpt_header = (GPT*)header_mapping.ptr();
    // dump: 
    log::log$("GPT Signature: {}", core::Str(gpt_header->signature, 8));
    log::log$("GPT Revision: {}", core::copy(gpt_header->revision));
    log::log$("GPT Size: {}", core::copy(gpt_header->size));
    log::log$("GPT Partition Entry LBA: {}", core::copy(gpt_header->lba_start_guid_partition_entry));
    log::log$("GPT Partition Count: {}", core::copy(gpt_header->partition_count));
    log::log$("GPT Partition Entry Size: {}", core::copy(gpt_header->partition_entry_size));

    result.header = *gpt_header;

    // parse partition entries
    size_t partition_entries_size = gpt_header->partition_count * gpt_header->partition_entry_size;
    size_t partition_entries_sectors = math::alignUp((partition_entries_size), (size_t)512);
    Wingos::MemoryAsset partition_entries_asset = (Wingos::Space::self().allocate_physical_memory(math::alignUp(partition_entries_sectors, (size_t)4096)));
    auto read_entries_res = connection.read(partition_entries_asset, gpt_header->lba_start_guid_partition_entry, partition_entries_sectors);
    Wingos::VirtualMemoryAsset partition_entries_mapping = Wingos::Space::self().map_memory(partition_entries_asset, ASSET_MAPPING_FLAG_READ | ASSET_MAPPING_FLAG_WRITE);
    GPTPartitionEntries* partition_entries = (GPTPartitionEntries*)partition_entries_mapping.ptr();

    for (size_t i = 0; i < gpt_header->partition_count; i++)
    {
        GPTPartitionEntries* entry = (GPTPartitionEntries*)((uint8_t*)partition_entries + i * gpt_header->partition_entry_size);
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
        core::WStr part_name;
        for (size_t j = 0; j < 36; j++)
        {
            if (entry->name[j] == 0)
            {
                break;
            }
            part_name.append(core::Str((char*)&(entry->name[j]), 1)); // convert char16_t to char
        }
        log::log$("Found partition: {} (LBA {} - {})", part_name.view(), core::copy(entry->lba_start), core::copy(entry->lba_end));

        GPTDiskParseEntry parse_entry = {};
        parse_entry.name = core::move(part_name);
        parse_entry.entry = entry;
        result.entries.push(core::move(parse_entry));
        
    }


    return (result);
}