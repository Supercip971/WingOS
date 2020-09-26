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
echfs_file_header echfs::read_directory_entry(uint64_t entry)
{

    uint64_t *another_buffer = (uint64_t *)malloc(512);
    uint64_t current_entry = 0;
    for (uint64_t i = 0; i < header.main_directory_length; i++)
    {

        read_block(main_dir_start + i, (uint8_t *)another_buffer);
        for (uint64_t j = 0; j < header.block_length / sizeof(echfs_file_header); j++)
        {
            echfs_file_header *cur_header = reinterpret_cast<echfs_file_header *>((uint64_t)another_buffer + (j * sizeof(echfs_file_header)));
            if (current_entry == entry)
            {
                echfs_file_header ret = *cur_header;
                free(another_buffer);
                return ret;
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
                    continue;
                }
            }
            current_entry++;
        }
    }
end:
    return {0};
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

    main_dir_start = ((header.block_count * sizeof(uint64_t) + header.block_length - 1) / header.block_length) + 16;
    uint64_t start_main_dir = ((header.block_count * sizeof(uint64_t) + header.block_length - 1) / header.block_length) + 16;
    log("echfs", LOG_INFO) << "end of allocation bloc : " << start_main_dir;
    uint64_t entry = 0;

    uint64_t entry_t = 0;
    while (true)
    {
        echfs_file_header head = read_directory_entry(entry_t);

        if (head.parent_id == 0)
        {
            break;
        }
        if (head.file_type == 1)
        {

            log("echfs", LOG_DEBUG) << "folder : " << entry_t;
            log("echfs", LOG_INFO) << head.file_name;
            log("echfs", LOG_INFO) << "parent id    :" << head.parent_id;
            log("echfs", LOG_INFO) << "directory id :" << head.starting_block;
        }
        else
        {

            log("echfs", LOG_DEBUG) << "file : " << entry_t;
            log("echfs", LOG_INFO) << head.file_name;
            log("echfs", LOG_INFO) << "parent id      :" << head.parent_id;
            log("echfs", LOG_INFO) << "size           :" << head.size;
            log("echfs", LOG_INFO) << "starting block :" << head.starting_block;
        }

        entry_t++;
    }

    log("echfs", LOG_INFO) << "found file : init_fs/test_directory/test_another.txt" << find_file("init_fs/test_directory/test_another.txt");

    log("echfs", LOG_INFO) << "reading file "
                           << "init_fs/test_directory/test_another.txt";
    uint8_t *f = this->ech_read_file("init_fs/test_directory/test_another.txt");
    log("echfs", LOG_INFO) << (char *)f;
}
char path_delimitor[] = "/";
uint64_t echfs::get_folder(uint64_t folder_id)
{

    uint64_t entry_t = 0;
    while (true)
    {

        echfs_file_header head = read_directory_entry(entry_t);
        if (head.file_type == 1)
        {
            if (head.starting_block == folder_id)
            {
                return entry_t;
            }
        }

        if (head.parent_id == 0 /* reach the end */)
        {
            return -1;
        }
        entry_t++;
    }
    return -1;
}
uint64_t echfs::get_simple_file(const char *name, uint64_t forced_parent)
{
    uint64_t entry_t = 0;
    while (true)
    {

        echfs_file_header head = read_directory_entry(entry_t);

        if (strncmp(head.file_name, name, strlen(name)) == 0)
        {
            if (forced_parent != -1)
            {
                if (head.parent_id == forced_parent)
                {
                    return entry_t;
                }
            }
            else
            {

                return entry_t;
            }
        }

        if (head.parent_id == 0 /* reach the end */)
        {
            return -1;
        }

        entry_t++;
    }
}
uint64_t echfs::find_file(const char *path)
{
    if (strlen(path) > 200)
    {
        log("echfs", LOG_ERROR) << "with echfs file path can't get larger than 200";
        return -1;
    }
    char *buffer_temp = (char *)malloc(201);
    char *path_copy = (char *)malloc(201);
    memzero(path_copy, 201);
    for (int i = 0; i < strlen(path); i++)
    {
        path_copy[i] = path[i];
    }
    log("echfs", LOG_INFO) << "searching for " << path_copy;

    memzero(buffer_temp, 201);
    bool is_end = false;
    echfs_file_header current_header;

    uint64_t current_parent = 0xffffffffffffffff;

redo: // yes goto are bad but if someone has a solution i take it ;)

    memzero(buffer_temp, 201);

    for (uint64_t i = 0; *path_copy != '/'; path_copy++)
    {
        if (*path_copy == 0)
        {
            buffer_temp[i] = 0;
            is_end = true;
            break;
        }

        buffer_temp[i++] = *path_copy;
        buffer_temp[i] = 0;
    }

    path_copy++;

    log("echfs", LOG_INFO) << "checking for " << buffer_temp;

    if (!is_end)
    {
        uint64_t next_idx = get_simple_file(buffer_temp, current_parent);
        if (next_idx != -1)
        {
            current_header = read_directory_entry(next_idx);
            if (current_header.file_type == 0)
            {

                log("echfs", LOG_ERROR) << "entry use a file as a directory " << buffer_temp;
                free(buffer_temp);
                return -1;
            }
            current_parent = current_header.starting_block;
            goto redo;
        }
        else
        {

            log("echfs", LOG_ERROR) << "entry not found" << buffer_temp;
            free(buffer_temp);
            return -1;
        }
    }
    else
    {

        uint64_t next_idx = get_simple_file(buffer_temp, current_parent);
        if (next_idx != -1)
        {
            current_header = read_directory_entry(next_idx);
            log("echfs", LOG_INFO) << "file found ! ";
            free(buffer_temp);
            return next_idx;
        }
        else
        {

            log("echfs", LOG_ERROR) << "entry not found" << buffer_temp;
            free(buffer_temp);
            return -1;
        }
    }
    return -1;
}
// read a file and redirect it to an address
// return 0 when not found
uint8_t *echfs::read_file(const char *path)
{
    return ech_read_file(path);
}
uint8_t *echfs::ech_read_file(const char *path)
{
    log("echfs", LOG_INFO) << "reading file " << path;
    echfs_file_header file_to_read_header = read_directory_entry(find_file(path));
    if (file_to_read_header.file_type == 1)
    {
        log("echfs", LOG_ERROR) << "trying to read a folder";
        return nullptr;
    }
    log("echfs", LOG_INFO) << "reading file 1 " << path;
    uint64_t size_to_read = file_to_read_header.size;
    size_to_read /= header.block_length;
    size_to_read++;
    size_to_read *= header.block_length;
    uint64_t block_count_to_read = size_to_read / header.block_length;
    log("echfs", LOG_INFO) << "reading file size  " << size_to_read;
    uint8_t *data = (uint8_t *)malloc(size_to_read);
    uint64_t block_to_read = file_to_read_header.starting_block;
    for (uint64_t i = 0; i < block_count_to_read; i++)
    {
        read_block(block_to_read + i, data + (i * header.block_length));
    }

    return data;
}
