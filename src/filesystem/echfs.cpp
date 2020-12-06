
#include <arch/lock.h>
#include <arch/mem/memory_manager.h>
#include <arch/mem/physical.h>
#include <device/ata_driver.h>
#include <filesystem/echfs.h>
#include <kernel.h>
#include <logging.h>
#include <utility.h>
lock_type main_echfs_lock = {0};
echfs::echfs() : file_system()
{
}
void echfs::read_block(uint64_t block_id, uint8_t *buffer)
{
    ata_driver::the()->read((start_sec + (block_id * header.block_length)) / 512, header.block_length / 512, buffer);
}
void echfs::read_blocks(uint64_t block_id, uint64_t length, uint8_t *buffer)
{
    ata_driver::the()->read((start_sec + (block_id * header.block_length)) / 512, ((header.block_length) / 512) * length, buffer);
}
uint64_t *another_buffer = nullptr;
echfs_file_header echfs::read_directory_entry(uint64_t entry)
{
    if (another_buffer == nullptr)
    {
        another_buffer = (uint64_t *)malloc(header.block_length);
    }
    uint64_t current_entry = 0;
    uint64_t second_check_length = header.block_length / sizeof(echfs_file_header);
    for (uint64_t i = 0; i < header.main_directory_length; i++)
    {

        read_block(main_dir_start + i, (uint8_t *)another_buffer);
        for (uint64_t j = 0; j < second_check_length; j++)
        {
            echfs_file_header *cur_header = reinterpret_cast<echfs_file_header *>((uint64_t)another_buffer + (j * sizeof(echfs_file_header)));
            if (current_entry == entry)
            {
                echfs_file_header ret = *cur_header;
                return ret;
            }
            if (cur_header->parent_id == 0)
            {
                log("echfs", LOG_INFO) << "reached the end of me :(";
                return {0};
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
    return {0};
}

bool echfs::is_valid_echfs_entry(uint64_t start_sector)
{

    uint8_t *temp_buffer = (uint8_t *)malloc(512);
    ata_driver::the()->read(start_sector / 512, 1, temp_buffer);
    block0_header hdr = *reinterpret_cast<block0_header *>(temp_buffer);
    if (strncmp(hdr.echfs_signature, "_ECH_FS_", 8) != 0)
    {
        free(temp_buffer);
        return false;
    }
    free(temp_buffer);
    return true;
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

    // uint64_t int64_per_sector = 512 / sizeof(uint64_t);
    // uint64_t echfs_alloc_table = 16;

    main_dir_start = ((header.block_count * sizeof(uint64_t) + header.block_length - 1) / header.block_length) + 16;
    uint64_t start_main_dir = ((header.block_count * sizeof(uint64_t) + header.block_length - 1) / header.block_length) + 16;
    log("echfs", LOG_INFO) << "end of allocation bloc : " << start_main_dir;
    // uint64_t entry = 0;

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

    log("echfs", LOG_INFO) << "found file : init_fs/test_directory/test_another.txt" << find_file("init_fs/test_directory/test_another.txt").parent_id;

    log("echfs", LOG_INFO) << "reading file "
                           << "init_fs/test_directory/test_another.txt";
    uint8_t *f = this->ech_read_file("init_fs/test_directory/test_another.txt");
    log("echfs", LOG_INFO) << (char *)f;

    free(temp_buffer);
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
echfs_file_header echfs::get_directory_entry(const char *name, uint64_t forced_parent)
{
    uint64_t entry_t = 0;
    uint64_t name_length = strlen(name);

    uint64_t *temporary_buf = (uint64_t *)malloc(header.block_length);

    //  uint64_t current_entry = 0;
    uint64_t second_check_length = header.block_length / sizeof(echfs_file_header);
    for (uint64_t i = 0; i < header.main_directory_length; i++)
    {

        read_block(main_dir_start + i, (uint8_t *)temporary_buf);
        for (uint64_t j = 0; j < second_check_length; j++)
        {
            echfs_file_header *cur_header = reinterpret_cast<echfs_file_header *>((uint64_t)temporary_buf + (j * sizeof(echfs_file_header)));
            if (strncmp(cur_header->file_name, name, name_length) == 0)
            {
                if (forced_parent != (uint64_t)-1)
                {
                    if (cur_header->parent_id == forced_parent)
                    {
                        echfs_file_header r = *cur_header;
                        free(temporary_buf);
                        return r;
                    }
                }
                else
                {
                    echfs_file_header r = *cur_header;
                    free(temporary_buf);
                    return r;
                }
            }
            if (cur_header->parent_id == 0 /* reach the end */)
            {
                free(temporary_buf);
                return {0};
            }

            entry_t++;
        }
    }
    free(temporary_buf);
    return {0};
}
uint64_t echfs::get_simple_file(const char *name, uint64_t forced_parent)
{
    uint64_t entry_t = 0;
    uint64_t name_length = strlen(name);

    uint64_t *temporary_buf = (uint64_t *)malloc(header.block_length);

    uint64_t second_check_length = header.block_length / sizeof(echfs_file_header);
    for (uint64_t i = 0; i < header.main_directory_length; i++)
    {

        read_block(main_dir_start + i, (uint8_t *)temporary_buf);
        for (uint64_t j = 0; j < second_check_length; j++)
        {
            echfs_file_header *cur_header = reinterpret_cast<echfs_file_header *>((uint64_t)another_buffer + (j * sizeof(echfs_file_header)));
            if (strncmp(cur_header->file_name, name, name_length) == 0)
            {
                if (forced_parent != (uint64_t)-1)
                {
                    if (cur_header->parent_id == forced_parent)
                    {
                        free(temporary_buf);
                        return entry_t;
                    }
                }
                else
                {

                    free(temporary_buf);
                    return entry_t;
                }
            }
            if (cur_header->parent_id == 0 /* reach the end */)
            {
                free(temporary_buf);
                return -1;
            }

            entry_t++;
        }
    }
    free(temporary_buf);
    return -1;
}
echfs_file_header echfs::find_file(const char *path)
{

    if (strlen(path) > 200)
    {
        log("echfs", LOG_ERROR) << "with echfs file path can't get larger than 200";
        return {0};
    }
    char *buffer_temp = (char *)malloc(256);
    char *path_copy = (char *)malloc(256);
    char *base_path_copy_addr = path_copy;
    memzero(path_copy, 255);

    memcpy(path_copy, path, strlen(path));
    log("echfs", LOG_INFO) << "searching for " << path_copy;

    memzero(buffer_temp, 255);
    bool is_end = false;
    // echfs_file_header current_header;

    uint64_t current_parent = 0xffffffffffffffff;
    uint64_t last_buffer_temp_used_length = 255;
redo: // yes goto are bad but if someone has a solution i take it ;)

    memzero(buffer_temp, last_buffer_temp_used_length);
    last_buffer_temp_used_length = 0;
    for (uint64_t i = 0; *path_copy != '/'; path_copy++)
    {
        if (*path_copy == 0)
        {
            buffer_temp[i] = 0;
            is_end = true;
            break;
        }
        last_buffer_temp_used_length++;
        buffer_temp[i++] = *path_copy;
        buffer_temp[i] = 0;
    }
    last_buffer_temp_used_length++;
    path_copy++;

    log("echfs", LOG_INFO) << "checking for " << buffer_temp;

    if (!is_end)
    {
        echfs_file_header current_header = get_directory_entry(buffer_temp, current_parent);
        if (current_header.parent_id != 0)
        {
            if (current_header.file_type == 0)
            {

                log("echfs", LOG_ERROR) << "entry use a file as a directory " << buffer_temp;
                free(buffer_temp);
                free(base_path_copy_addr);
                return {0};
            }
            current_parent = current_header.starting_block;
            goto redo;
        }
        else
        {

            log("echfs", LOG_ERROR) << "entry not found" << buffer_temp;
            free(buffer_temp);
            free(base_path_copy_addr);
            return {0};
        }
    }
    else
    {

        echfs_file_header current_header = get_directory_entry(buffer_temp, current_parent);
        if (current_header.parent_id != 0)
        {
            log("echfs", LOG_INFO) << "file found ! ";
            free(buffer_temp);
            free(base_path_copy_addr);
            return current_header;
        }
        else
        {

            log("echfs", LOG_ERROR) << "entry not found" << buffer_temp;
            free(buffer_temp);
            free(base_path_copy_addr);
            return {0};
        }
    }
    free(buffer_temp);
    free(base_path_copy_addr);
    return {0};
}
// read a file and redirect it to an address
// return 0 when not found

uint64_t echfs::get_file_length(const char *path)
{
    echfs_file_header file_to_read_header = (find_file(path));

    //  uint64_t size_to_read = file_to_read_header.size;
    return file_to_read_header.size;
}
uint8_t *echfs::ech_read_file(const char *path)
{
    flock(&main_echfs_lock);
    log("echfs", LOG_INFO) << "reading file " << path;
    echfs_file_header file_to_read_header = (find_file(path));
    if (file_to_read_header.file_type == 1)
    {
        log("echfs", LOG_ERROR) << "trying to read a folder";
        unlock(&main_echfs_lock);
        return nullptr;
    }
    log("echfs", LOG_INFO) << "reading file 1 " << path;
    uint64_t size_to_read = file_to_read_header.size;
    size_to_read /= header.block_length;
    size_to_read++;
    size_to_read *= header.block_length;
    uint64_t block_count_to_read = size_to_read / header.block_length;
    uint8_t *data = (uint8_t *)malloc(size_to_read);
    log("echfs", LOG_INFO) << "reading file size  " << size_to_read;
    const uint64_t block_to_read = file_to_read_header.starting_block;

    read_blocks(block_to_read, block_count_to_read, data);

    log("echfs", LOG_INFO) << "readed file size  " << size_to_read;
    unlock(&main_echfs_lock);
    return data;
}
