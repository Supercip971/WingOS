#include <com.h>
#include <device/ata_driver.h>
#include <filesystem/partition/base_partition.h>
#include <io_device.h>
#include <logging.h>
#include <utils/liballoc.h>
base_partition::base_partition()
{
}

MBR_partition::MBR_partition()
{
}

void MBR_partition::init()
{
    log("mbr", LOG_DEBUG) << "loading mbr partition ....";
    uint8_t *temp_buffer = (uint8_t *)malloc(1024);
    get_io_device(0)->read(temp_buffer, 1, 0);

    entry = (mbr_entry *)(temp_buffer + 0x1BE);

    bool is_at_least_one_valid = false;
    for (int i = 0; i < 4; i++)
    {
        log("mbr", LOG_DEBUG) << "loading mbr partition" << i;
        if (!entry[i].type)
        {
            log("mbr", LOG_INFO) << "not valid partition" << i;
        }
        else
        {
            log("mbr", LOG_INFO) << "valid partition" << i;
            log("mbr", LOG_INFO) << "first sector" << entry[i].first_sect;
            log("mbr", LOG_INFO) << "status" << entry[i].status;
            log("mbr", LOG_INFO) << "type" << entry[i].type;
            log("mbr", LOG_INFO) << "sector count" << entry[i].sect_count;
            is_at_least_one_valid = true;
        }
    }
    if (!is_at_least_one_valid)
    {
        log("mbr", LOG_ERROR) << "no valid partition found :(";
        return;
    }
}
uint8_t MBR_partition::get_parition_count()
{
    uint8_t partition_count = 0;
    for (int i = 0; i < 4; i++)
    {
        if (!entry[i].type)
        {
        }
        else
        {
            partition_count++;
        }
    }
    return partition_count;
}

uint64_t MBR_partition::get_partition_start(uint8_t partition_id)
{
    return entry[partition_id].first_sect * 512;
}
uint64_t MBR_partition::get_partition_length(uint8_t partition_id)
{
    return entry[partition_id].sect_count * 512;
}
