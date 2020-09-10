#include <arch/mem/liballoc.h>
#include <com.h>
#include <device/ata_driver.h>
#include <filesystem/partition/base_partition.h>
base_partition::base_partition()
{
}

MBR_partition::MBR_partition()
{
}

void MBR_partition::init()
{
    printf("loading mbr partition ....");
    uint8_t *temp_buffer = (uint8_t *)malloc(512);
    ata_driver::the()->read(0, 2, temp_buffer);

    entry = (mbr_entry *)(temp_buffer + 0x1BE);

    bool is_at_least_one_valid = false;
    for (int i = 0; i < 4; i++)
    {
        if (!entry[i].type)
        {
            printf("not valid partition %x \n", i);
        }
        else
        {
            printf("valid partition %x \n", i);
            printf("partition data : \n");
            printf("first sector %x \n ", entry[i].first_sect);
            printf("status %x \n ", entry[i].status);
            printf("type %x \n ", entry[i].type);
            printf("sector count %x \n ", entry[i].sect_count);
            is_at_least_one_valid = true;
        }
    }
    if (!is_at_least_one_valid)
    {
        printf("no valid partition found [MBR] :( \n");
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

        }else{
            partition_count++;
        }
    }
    return partition_count;
}

uint64_t MBR_partition::get_partition_start(uint8_t partition_id)
{
    return entry[partition_id].first_sect * 512;
}
