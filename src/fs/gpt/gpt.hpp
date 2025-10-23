#pragma once 
#include <stdint.h>
#include <stddef.h>
#include <libcore/result.hpp>
#include <libcore/ds/vec.hpp>
#include <libcore/str_writer.hpp>


namespace Wingos 
{

  struct __attribute__((packed)) GPT 
  {
    char signature[8];
    uint32_t revision; 
    uint32_t size; 
    uint32_t checksum_crc32;
    uint32_t _reserved; 
    uint64_t lba_header; 
    uint64_t lba_alternative; 
    uint64_t first_usable_block;
    uint64_t last_usable_block; 
    uint8_t disk_guid[16];
    uint64_t lba_start_guid_partition_entry;
    uint32_t partition_count;
    uint32_t partition_entry_size;
    uint32_t partition_entry_crc32;

  };

  struct __attribute__((packed))GPTPartitionEntries 
  {
    uint8_t part_type_guid[16]; // 0 unused
    uint8_t part_uuid_guid[16];
    uint64_t lba_start; 
    uint64_t lba_end; 
    uint64_t attributes;
    char16_t name[];
  };


  struct GPTDiskParseEntry 
  {
    core::WStr name; 
    GPTPartitionEntries* entry;
  };


  

  struct GPTDiskParseResult 
  {
    GPT header; 
    core::Vec<GPTDiskParseEntry> entries; 
  };


  core::Result<GPTDiskParseResult> parse_gpt(core::Str& device);

};
