#include <arch/mem/liballoc.h>
#include <device/ata_driver.h>
#include <filesystem/echfs.h>
#include <kernel.h>
#include <loggging.h>
#include <utility.h>
echfs::echfs()
{
}
void echfs::read_block(uint64_t block_id, uint8_t *buffer)
{

    ata_driver::the()->read((start_sec + (block_id * header.block_length)) / 512, header.block_length / 512, buffer);
}
echfs_file_header *echfs::read_directory_entry(uint64_t entry)
{

    uint64_t *another_buffer = (uint64_t *)malloc(512);
    uint64_t current_entry = 0;
    for (uint64_t i = 0; i < header.main_directory_length; i++)
    {

        read_block(main_dir_start + i, (uint8_t *)another_buffer);
        for (uint64_t j = 0; j < header.block_length / sizeof(echfs_file_header); j++)
        {
            echfs_file_header *cur_header = reinterpret_cast<echfs_file_header *>(another_buffer + (j * sizeof(echfs_file_header)));
            if (current_entry == entry)
            {
                return cur_header;
            }
            if (cur_header->parent_id == 0)
            {
                log("echfs", LOG_INFO) << "reached the end of me :(";
                goto end;
            }
            else
            {

                if (cur_header->parent_id == 0xFFFFFFFFFFFFFFFE)
                {
                    break;
                }
                log("echfs", LOG_INFO) << "dir found !";
                log("echfs", LOG_INFO) << cur_header->file_name;
                log("echfs", LOG_INFO) << "parent id" << cur_header->parent_id;
                log("echfs", LOG_INFO) << "size" << cur_header->size;
            }
            current_entry++;
        }
    }
end:
    return nullptr;
}
void echfs::init(uint64_t start_sector, uint64_t sector_count)
{
    log("echfs", LOG_DEBUG) << "loading echfs";
    log("echfs", LOG_INFO) << "echfs start :" << start_sector << " end :" << (start_sector + sector_count);
    uint8_t *temp_buffer = (uint8_t *)malloc(512);
    ata_driver::the()->read(start_sector / 512, 1, temp_buffer);
    start_sec = start_sector;
    header = *reinterpret_cast<block0_header *>(temp_buffer);
    if (strncmp(header.echfs_signature, "_ECH_FS_", 8) != 0)
    {
        log("echfs", LOG_ERROR) << "not valid echfs partition";
        return;
    }
    log("echfs", LOG_INFO) << " echfs file system info :";
    log("echfs", LOG_INFO) << "echfs header          : " << header.echfs_signature;
    log("echfs", LOG_INFO) << "echfs block count     : " << header.block_count;
    log("echfs", LOG_INFO) << "echfs block length    : " << header.block_length;
    log("echfs", LOG_INFO) << "echfs main dir length : " << header.main_directory_length;
    log("echfs", LOG_INFO) << "dir size              : " << sizeof(echfs_file_header);
    log("echfs", LOG_INFO) << "total dir count       : " << (header.main_directory_length * header.block_length) / sizeof(echfs_file_header);

    uint64_t *another_buffer = (uint64_t *)malloc(512);

    uint64_t int64_per_sector = 512 / sizeof(uint64_t);
    uint64_t echfs_alloc_table = 16;
    /* while (true)
    {
        read_block(echfs_alloc_table, (uint8_t *)another_buffer);
        for (int i = 0; i < int64_per_sector; i++)
        {
            if (another_buffer[i] == 0xFFFFFFFFFFFFFFFF)
            {
                log("echfs", LOG_INFO) << "end of echfs alloc table at block: " << echfs_alloc_table;
                goto endtable;
                break;
            }
            else if (another_buffer[i] == 0)
            {
                //    log("echfs", LOG_INFO) << "free alloc table at block: " << echfs_alloc_table;
            }
        }
        echfs_alloc_table++;
    }
endtable:;*/
    main_dir_start = ((header.block_count * sizeof(uint64_t) + header.block_length - 1) / header.block_length) + 16;
    uint64_t start_main_dir = ((header.block_count * sizeof(uint64_t) + header.block_length - 1) / header.block_length) + 16;
    log("echfs", LOG_INFO) << "end of allocation bloc : " << start_main_dir;
    //uint64_t dir_block_length = (header.main_directory_length * header.block_length) ;
    uint64_t entry = 0;
    for (uint64_t i = 0; i < header.main_directory_length; i++)
    {
        memzero(another_buffer, 512);
        read_block(start_main_dir + i, (uint8_t *)another_buffer);
        for (uint64_t j = 0; j < header.block_length / sizeof(echfs_file_header); j++)
        {
            entry++;
            echfs_file_header *cur_header = reinterpret_cast<echfs_file_header *>((uint64_t)another_buffer + (j * sizeof(echfs_file_header)));
            if (cur_header->parent_id == 0 || cur_header->parent_id == 0xFFFFFFFFFFFFFFFE)
            {
                goto end; // yeah i should use something else
            }
            else
            {
                if (
                    cur_header->starting_block == 0)
                {
                    continue;
                }
                log("echfs", LOG_DEBUG) << "entry : " << entry;
                log("echfs", LOG_INFO) << cur_header->file_name;
                log("echfs", LOG_INFO) << "parent id" << cur_header->parent_id;
                log("echfs", LOG_INFO) << "size" << cur_header->size;
                log("echfs", LOG_INFO) << "type" << cur_header->file_type;
                log("echfs", LOG_INFO) << "starting block :" << cur_header->starting_block;
            }
        }
    }
end:;
}

// read a file and redirect it to an address
// return 0 when not found
uint8_t *echfs::read_file(const char *path)
{
}
