#include "gpt.hpp"
#include <libcore/result.hpp>
#include <protocols/disk/disk.hpp>
#include <libcore/fmt/log.hpp>

core::Result<Wingos::GPTDiskParseResult> Wingos::parse_gpt(core::Str& device)
{
    Wingos::GPTDiskParseResult result = {};

    auto connection = try$(prot::DiskConnection::connect(device));

    Wingos::MemoryAsset header_asset = (Wingos::Space::self().allocate_physical_memory(4096));


    
    auto read_res = connection.read(header_asset, 0, 512);

    Wingos::VirtualMemoryAsset header_mapping = Wingos::Space::self().map_memory(header_asset, ASSET_MAPPING_FLAG_READ | ASSET_MAPPING_FLAG_WRITE);

    for(size_t i = 0; i < 512; i++)
    {
        log::log$("GPT Header byte {}: {}", i, ((uint8_t*)header_mapping.ptr())[i] | fmt::FMT_HEX);
    }

    return result;
}