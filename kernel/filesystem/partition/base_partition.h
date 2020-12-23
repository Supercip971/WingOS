#pragma once
#include <stdint.h>
class base_partition
{
    static base_partition *main_disk_partition;

public:
    base_partition();

    virtual void init() = 0;
    virtual uint8_t get_parition_count() = 0;
    virtual uint64_t get_partition_start(uint8_t partition_id) = 0;
    virtual uint64_t get_partition_length(uint8_t partition_id) = 0;
};

struct mbr_entry
{
    uint8_t status;
    uint8_t chs_first_sect[3];
    uint8_t type;
    uint8_t chs_last_sect[3];
    uint32_t first_sect;
    uint32_t sect_count;
} __attribute__((packed));

class MBR_partition : public base_partition
{
    mbr_entry *entry;

public:
    MBR_partition();
    virtual void init() override;
    virtual uint8_t get_parition_count() override;
    virtual uint64_t get_partition_start(uint8_t partition_id) override;
    virtual uint64_t get_partition_length(uint8_t partition_id) override;
};
