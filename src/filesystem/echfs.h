#ifndef ECHFS_H
#define ECHFS_H

#include <filesystem/file_system.h>
struct block0_header
{
    uint8_t assembly_jump[4];
    char echfs_signature[8];
    uint64_t block_count;
    uint64_t main_directory_length;
    uint64_t block_length;
} __attribute__((packed));
struct echfs_file_header
{
    uint64_t parent_id;
    uint8_t file_type;
    char file_name[201];

    uint64_t unix_access_time;
    uint64_t unix_modify_time;

    uint16_t permissions;
    uint16_t owner_id;
    uint16_t group_id;

    uint64_t unix_create_time;

    uint64_t starting_block; // Or directory id if it's a directory
    uint64_t size;

} __attribute__((packed));

class echfs : public file_system
{
    uint64_t start_sec;
    block0_header header;
    void read_block(uint64_t block_id, uint8_t *buffer);
    uint64_t main_dir_start;
    echfs_file_header read_directory_entry(uint64_t entry);
    echfs_file_header get_directory_entry(const char *name, uint64_t forced_parent = -1) __attribute__((__target__("no-sse")));
    echfs_file_header find_file(const char *path) __attribute__((__target__("no-sse")));
    uint64_t get_folder(uint64_t folder_id);
    uint64_t get_simple_file(const char *name, uint64_t forced_parent = -1);

public:
    echfs();
    virtual void init(uint64_t start_sector, uint64_t sector_count) override;

    // read a file and redirect it to an address
    // return 0 when not found
    uint8_t *ech_read_file(const char *path);
    virtual uint8_t *read_file(const char *path) override;

    // mainly not implemented
    //    virtual fs_file *get_file(const char *path) override;

    // these function aren't supported for the moment

    virtual const char *get_filesystem_type_name() override
    {
        return "echfs";
    }
};

#endif // ECHFS_H
