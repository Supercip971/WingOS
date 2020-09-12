#pragma once

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

    // read a file and redirect it to an address
    // return 0 when not found
    virtual uint8_t *read_file(const char *path) = 0;

    // mainly not implemented

    // these function aren't supported for the moment

    virtual const char *get_filesystem_type_name()
    {
        return "null_fs";
    }
};
