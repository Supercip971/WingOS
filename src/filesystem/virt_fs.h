#pragma once
#include <stdint.h>
#define to_fs(a) (virt_fs *)(&a)

class virt_file
{
public:
    virtual const char *get_path() = 0;
    virtual uint64_t get_size() = 0;

protected:
    uint64_t f_id;
    uint64_t address;
    const char *fixed_path;
};
enum filesystem_type_int
{
    NULL_fs = 0,
    custom_fs = 1,
    tar_fs = 2

};
class virt_fs
{
public:
    virt_fs();
    virtual void init(uint64_t module_start, uint64_t module_end) = 0;

    // read a file and redirect it to an address
    // return 0 when not found
    virtual uint64_t read_file(const char *path) = 0;

    // read a file and redirect it to an address wich point to a copy of the file
    // return 0 when not found
    virtual uint64_t read_file_copy(const char *path) = 0;

    // mainly not implemented
    virtual virt_file *get_file(const char *path) = 0;

    // these function aren't supported for the moment

    virtual const char *get_filesystem_type_name()
    {
        return "null_fs";
    }

    virtual filesystem_type_int get_filesystem_type_type()
    {
        return NULL_fs;
    }

protected:
    uint64_t fs_module_addr;
    uint64_t fs_module_end;
};

class global_fs
{
    virt_fs *main_fs;

public:
    void set_fs(virt_fs *file_system)
    {
        main_fs = file_system;
    }
    virt_fs *get_fs()
    {
        return main_fs;
    }

    static global_fs *the();
};
