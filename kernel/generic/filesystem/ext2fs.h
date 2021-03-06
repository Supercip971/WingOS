#ifndef EXTFS_H
#define EXTFS_H
#include <filesystem/file_system.h>
#define EXT2FS_SIGNATURE 0xef53
enum fs_state
{
    STATE_CLEAN = 1,
    STATE_ERROR = 2,
};
enum fs_error_handle
{
    HANDLE_IGNORE = 1,
    HANDLE_REMOUNT_RO = 2,
    HANDLE_PANIC = 3,
};
enum ext2fs_optinal_feature_value
{
    PREALOC_BLOCK = 0x1,
    AFS_INODE = 0x2,
    JOURNAL_SUPPORT = 0x4,
    EXTENDED_INODE_ATTRIB = 0x8,
    RESIZABLE_FS = 0x10,
    INDEXED_DIRECTORY = 0x20
};
enum ext2fs_read_only_feature_value
{
    SPARSE_DESC_TABLE = 0x1,
    LONG_MODE_FILE_SIZE = 0x2,
    BINARY_TREE_DIRECTORY_CONTENT = 0x4,
};
enum ext2fs_required_feature_value
{
    COMPRESSION = 0x1,
    DIRECTORY_TYPE_FIELD = 0x2,
    JOURNAL_REPLAY_NEEDED = 0x4,
    JOURNAL_DEVICE_NEEDED = 0x8
};

struct ext2fs_superblock
{
    uint32_t inode_count;
    uint32_t block_count;
    uint32_t block_reserved;
    uint32_t unallocated_block_count;
    uint32_t unallocated_inode_count;
    uint32_t superblock_id;
    uint32_t sblock_size;
    uint32_t fragment_size;
    uint32_t block_count_per_group;
    uint32_t fragment_per_block_group;
    uint32_t inode_count_per_group;
    uint32_t last_mnt_time;
    uint32_t last_write_time;

    uint16_t mount_count;
    uint16_t mount_consistency_check;
    uint16_t signature;
    uint16_t state;
    uint16_t error_action;
    uint16_t minor_version;

    uint32_t last_check_time;
    uint32_t last_check_time_interval;
    uint32_t osid;
    uint32_t version;
    uint16_t uid_for_reserved_block;
    uint16_t uid_for_reserved_group;
    uint32_t first_free_inode;
    uint16_t inode_size;
    uint16_t superblock_current_block;
    uint32_t optional_features;
    uint32_t required_features;
    uint32_t read_only_features;
    uint8_t fs_id[16];
    uint8_t volume_name[16];
    uint8_t last_mounted_path[64];
    uint32_t compression_algorithm;
    uint8_t prealocate_block_count_file;
    uint8_t prealocate_block_count_directory;
    uint16_t unused;
    uint8_t journal_id[16];
    uint32_t journal_inode;
    uint32_t journal_device;
    uint32_t journal_inode_orphaned;
} __attribute__((packed));

struct ext2fs_inode_structure
{
    uint16_t type;
    uint16_t uid;

    uint32_t lower_size;
    uint32_t last_access_time;
    uint32_t created_time;
    uint32_t last_modified_time;
    uint32_t deleted_time;

    uint16_t group_id;
    uint16_t hard_link_count;

    uint32_t block_count;
    uint32_t flag;
    uint32_t os_specific;
    uint32_t block_ptr[12];
    uint32_t singly_indirect_block_ptr;
    uint32_t doubly_indirect_block_ptr;
    uint32_t triply_indirect_block_ptr;
    uint32_t generation_num;

    uint32_t extended_attribute;
    uint32_t directory_acl;

    uint32_t fragment_addr;

    uint8_t os_specifics[12];
} __attribute__((packed));
class ext2fs_inode
{
public:
    ext2fs_inode_structure strct;
    size_t id;
    size_t last_free_gid;
    ext2fs_inode()
    {
        last_free_gid = 0;
        id = 0;
        strct = {0};
    };
    ext2fs_inode(ext2fs_inode &other);
    ext2fs_inode(const ext2fs_inode &other);
    const size_t get_size() { return strct.lower_size; };
    const bool is_valid() { return id != 0; };
};

enum ext2fs_directory_type
{
    UNKNOWN_DIR = 0,
    REGULAR_DIR = 1,
    DIRRECTORY_DIR = 2,
    CHAR_DEVICE_DIR = 3,
    BLOCK_DEVICE_DIR = 4,
    FIFO_DIR = 5,
    SOCKET_DIR = 6,
    SYMBOLIC_LINK = 7,
};

struct ext2fs_directory
{
    uint32_t inode_dir;
    uint16_t directory_size;
    uint8_t directory_name_length;
    uint8_t type;
    char name[255];
} __attribute__((packed));
struct ext2fs_block_group_descriptor
{
    uint32_t block_bitmap;
    uint32_t inode_bitmap;
    uint32_t inode_table;
    uint16_t free_block;
    uint16_t free_inode;
    uint16_t dir_count;
    uint8_t reserved[14];
} __attribute__((packed));

class ext2fs : public file_system
{
    utils::lock_type ext_lock;
    uint64_t inode_per_group;
    constexpr uint64_t get_group_from_inode(uint64_t inode);
    constexpr uint64_t get_local_group_inode_from_inode(uint64_t inode);
    ext2fs_superblock super_block;
    ext2fs_inode root_inode;
    uint64_t block_group_descriptor_table;
    uint64_t block_size;
    uint64_t offset; // offset with the partition

    ext2fs_block_group_descriptor read_group_from_inode(uint64_t inode);
    void write_group_from_inode(uint64_t inode, ext2fs_block_group_descriptor group);

    ext2fs_block_group_descriptor read_group_from_group_id(uint64_t gid);
    void write_group_from_group_id(uint64_t gid, ext2fs_block_group_descriptor group);
    void clear_block(size_t block_addr);
    bool inode_read(void *buffer, uint64_t cursor, uint64_t count, ext2fs_inode parent);
    bool inode_write(const void *buffer, uint64_t cursor, uint64_t count, ext2fs_inode parent);
    void read_block(uint64_t block_id, uint8_t *buffer);
    void read_blocks(uint64_t block_id, uint64_t length, uint8_t *buffer);
    uint64_t get_folder(uint64_t folder_id);
    uint64_t get_simple_file(const char *name, uint64_t forced_parent = -1);
    uint32_t *create_inode_block_map(ext2fs_inode inode_struct);
    ext2fs_inode get_inode(uint64_t inode);
    bool write_inode(ext2fs_inode inode);
    uint8_t *ext_read_file(const char *path);
    ext2fs_inode get_file(const char *path);
    ext2fs_inode find_subdir(ext2fs_inode inode_struct, const char *name);
    void print_ext2_feature();
    uint64_t get_inode_block_map(ext2fs_inode inode_struct, uint64_t block_id);
    void list_sub_directory(ext2fs_inode inode, int offset);
    void resize_file(ext2fs_inode &inode, uint64_t new_size);

    uint64_t alloc_block_for_inode(ext2fs_inode &inode);

    void add_inode_block_map(ext2fs_inode &inode_struct, uint32_t block_addr);

public:
    ext2fs();
    virtual void init(uint64_t start_sector, uint64_t sector_count) override;
    virtual bool exist(const char *path) override;
    // read a file and redirect it to an address
    // return 0 when not found
    virtual uint64_t get_file_length(const char *path) override;
    virtual uint8_t *read_file(const char *path) override
    {
        return ext_read_file(path);
    }

    virtual uint64_t read_file(const char *path, uint64_t at, uint64_t size, uint8_t *buffer) override;
    virtual uint64_t write_file(const char *path, uint64_t at, uint64_t size, const uint8_t *buffer) override;
    // mainly not implemented
    //    virtual fs_file *get_file(const char *path) override;

    // these function aren't supported for the moment

    virtual const char *get_filesystem_type_name() override
    {
        return "ext2fs";
    }
    static bool is_valid_ext2fs_entry(uint64_t start_sector);
};

#endif // ECHFS_H
