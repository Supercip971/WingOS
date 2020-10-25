#pragma once
#include <filesystem/partition/base_partition.h>
#include <logging.h>
#include <stdint.h>
class fs_file
{
public:
    virtual const char *get_path() = 0;
    virtual uint64_t get_size() = 0;

protected:
    uint64_t f_id;
    uint64_t address;
    const char *fixed_path;
};
class file_system
{
public:
    file_system();
    virtual void init(uint64_t start_sector, uint64_t sector_count) = 0;

    virtual uint64_t get_file_length(const char *path) = 0;
    // read a file and redirect it to an address
    // return 0 when not found
    virtual uint8_t *read_file(const char *path)
    {
        log("fs", LOG_ERROR) << "invalid call of file system read file" << path;
        return nullptr;
    }

    // mainly not implemented

    // these function aren't supported for the moment

    virtual const char *get_filesystem_type_name() = 0;
};

class main_fs_system
{

    file_system *file_systems[4];
    base_partition *partition_system;
    uint64_t partition_count = 0;

public:
    file_system *from_partition(uint64_t partition_id);
    file_system *main_fs();
    void init_file_system();
    static main_fs_system *the();
};
