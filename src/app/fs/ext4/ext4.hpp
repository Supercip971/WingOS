#pragma once
#include <stdint.h>
#include <string.h>
#include "protocols/disk/disk.hpp"

struct [[gnu::packed]] Ext4Superblock
{
    uint32_t inodes_count; // used and free inode
    uint32_t blocks_count_lo;
    uint32_t super_blocks_count; // total blocks count reserved for superuser
    uint32_t free_blocks_count_lo;
    uint32_t free_inodes_count;
    uint32_t first_data_block;
    uint32_t log_block_size; // block size = 1024 << log_block_size
    uint32_t log_frag_size;
    uint32_t blocks_per_group;
    uint32_t frags_per_group;
    uint32_t inodes_per_group;
    uint32_t mtime;
    uint32_t wtime;
    uint16_t mount_count;
    uint16_t max_mount_count;
    uint16_t magic;
    uint16_t state;
    uint16_t errors;
    uint16_t minor_rev_level;
    uint32_t lastcheck;
    uint32_t checkinterval;
    uint32_t creator_os;
    uint32_t rev_level;
    uint16_t def_resuid;
    uint16_t def_resgid;
    // EXT2_DYNAMIC_REV superblock fields
    uint32_t first_ino;
    uint16_t inode_size;
    uint16_t block_group_nr;
    uint32_t feature_compat;
    uint32_t feature_incompat;
    uint32_t feature_ro_compat;
    uint8_t uuid[16];
    char volume_name[16];
    char last_mounted[64];
    uint32_t algo_bitmap;
    // Performance hints
    uint8_t prealloc_blocks;
    uint8_t prealloc_dir_blocks;
    uint16_t _reserved;
    // Journaling support
    uint8_t journal_uuid[16];
    uint32_t journal_inum;
    uint32_t journal_dev;
    uint32_t last_orphan;
    // Directory indexing support
    uint32_t hash_seed[4];
    uint8_t def_hash_version;
    uint8_t journal_backup_type;

    uint16_t desc_size;

    uint32_t default_mount_opts;
    uint32_t first_meta_bg;

    uint32_t mkfs_time;

    uint32_t journal_blocks[17];

    uint32_t blocks_count_hi;
    uint32_t free_blocks_count_hi;
    uint32_t reserved_blocks_count_hi;

    uint16_t min_inode_size;
    uint16_t min_inode_reserve_size;
    uint32_t misc_flags;

    uint16_t raid_stride;
    uint16_t mmp_interval;
};

enum Ext4ReqFeature : uint32_t
{
    EXT4_FEAT_COMPRESSION = 0x0001,
    EXT4_FEAT_FILETYPE = 0x0002,
    EXT4_FEAT_RECOVER = 0x0004,
    EXT4_FEAT_JOURNAL_DEV = 0x0008,
    EXT4_FEAT_META_BG = 0x0010,
    EXT4_FEAT_EXTENTS = 0x0040,
    EXT4_FEAT_64BIT = 0x0080,
    EXT4_FEAT_MMP = 0x0100,
    EXT4_FEAT_FLEX_BG = 0x0200,
    EXT4_FEAT_EA_INODE = 0x0400,
    EXT4_FEAT_DIRDATA = 0x1000,
    EXT4_FEAT_CSUM_SEED = 0x2000,
    EXT4_FEAT_LARGEDIR = 0x4000,
    EXT4_FEAT_INLINE_DATA = 0x8000,
    EXT4_FEAT_ENCRYPT = 0x10000,
    EXT4_FEAT_CASEFOLD = 0x20000,

};

struct [[gnu::packed]] Ext4BlockGroupDescriptor
{
    uint32_t block_bitmap;
    uint32_t inode_bitmap;
    uint32_t inode_table;
    uint32_t free_blocks_count;
    uint32_t free_inodes_count;
    uint16_t used_dirs_count;

    uint16_t block_group_feat;
    uint32_t block_addr_snapshot;
    uint16_t checksum_block_usage;
    uint16_t checksum_inode_usage;
    uint16_t free_inode_ext;
    uint16_t chk_sum;

    // 64 bit

    uint32_t high_block_bitmap;
    uint32_t high_inode_bitmap;
    uint32_t high_inode_table;
    uint16_t high_free_blocks_count;
    uint16_t high_free_inodes_count;
    uint16_t high_dir_count;
    uint16_t high_free_inodes_ext;

    uint32_t high_block_addr_snapshot;
    uint16_t high_chcksum_block_usage;
    uint16_t high_chcksum_inode_usage;
    uint32_t _reserved;
};


struct [[gnu::packed]] Ext4Inode
{
    uint16_t permission : 12;
    uint16_t file_type : 4;
    uint16_t uid;
    uint32_t size_lo;
    uint32_t atime;
    uint32_t ctime;
    uint32_t mtime;
    uint32_t dtime;
    uint16_t gid;
    uint16_t hard_links_count;
    uint32_t blocks_lo;
    uint32_t flags;
    uint32_t osd1;
    uint32_t block[15];
    uint32_t generation;
    uint32_t extended_attr_block;
    uint32_t size_high;
    uint32_t obso_faddr;
    
    uint8_t osd2[12];

    // more fields may follow
};


struct [[gnu::packed]] Ext4DirEntry
{
    uint32_t inode;
    uint16_t rec_len;
    uint8_t name_len;
    uint8_t file_type;
    char name[];
};
using BlockGroupId = uint64_t;
using InodeId = uint64_t;
using Blockid = uint64_t;


struct Ext4InodeRef 
{
    InodeId inode_id;
    Ext4Inode inode; // 
};
class Ext4Filesystem
{
    prot::DiskConnection disk;
    size_t start_lba;

    size_t block_desc_size;
    size_t end_lba;
    size_t disk_block_size = 512;

    Ext4Superblock superblock;


    size_t bgd_table_start_block = 0;

    Wingos::MemoryAsset disk_asset;
    Wingos::VirtualMemoryAsset mapped_disk_asset;

public:

    // use temp buffer
    core::Result<void*> read_block_tmp(size_t block_num); 
    core::Result<void> write_block_tmp(size_t block_num, void* data);

    core::Result<size_t> inode_read(Ext4InodeRef const &inode, Wingos::MemoryAsset& out, size_t len, size_t block);
    core::Result<void*> inode_read_tmp(Ext4InodeRef const &inode, size_t block);


    core::Result<void> inode_write(Ext4InodeRef& inode, Wingos::MemoryAsset& out, size_t len, size_t block);
    core::Result<void> inode_write_tmp(Ext4InodeRef& inode, size_t block, void* data);


    core::Result<BlockGroupId> find_available_group_for_alloc(BlockGroupId start_from);

    core::Result<uint64_t> allocate_block(BlockGroupId bg_id);

    core::Result<void> inode_add_block(Ext4InodeRef& inode);


    core::Result<uint64_t> inode_find_block(Ext4InodeRef const& inode, size_t block);


    core::Result<void> dump_subdir(Ext4InodeRef const& dir_inode, int depth);





    size_t block_size () const {
        return (1024 << superblock.log_block_size);
    }
    // use temp buffer
    

    
    BlockGroupId blockgroup_from_inode(InodeId inode) const 
    {
        return (inode - 1) / superblock.inodes_per_group;
    }

    size_t blockgroup_inode_index(InodeId inode) const
    {
        return (inode - 1) % superblock.inodes_per_group;
    }

    size_t inode_containing_block(InodeId inode) const
    {
        size_t containing_block = blockgroup_inode_index(inode) * superblock.inode_size;
        return containing_block / block_size();
    }    


    core::Result<Ext4BlockGroupDescriptor> read_blockgroup_descriptor(BlockGroupId bg_id);

    core::Result<void> write_blockgroup_descriptor(BlockGroupId bg_id, Ext4BlockGroupDescriptor const &bgd);


    core::Result<Ext4InodeRef> read_inode(InodeId inode);
    core::Result<void> write_inode(InodeId inode, Ext4Inode const &data);

    static core::Result<Ext4Filesystem> initialize(prot::DiskConnection disk_conn, size_t start_lba, size_t end_lba)
    {
        log::log$("ext4: initializing ext4 filesystem...");
        Ext4Filesystem fs;
        fs.disk = (disk_conn);
        fs.start_lba = start_lba;
        fs.end_lba = end_lba;
        fs.disk_block_size = 512;


        fs.bgd_table_start_block = 0;
        // fixme: correctly setup disk block size
        Wingos::VirtualMemoryAsset vasset = try$(fs.disk.read_buf(start_lba + (0x400 / fs.disk_block_size), fs.disk_block_size));

        fs.superblock = *(Ext4Superblock *)vasset.ptr();

        if (fs.superblock.magic != 0xEF53)
        {
            return "not an ext4 filesystem (invalid magic)";
        }

        log::log$("ext4: detected ext4 filesystem with {} inodes and {} blocks", core::copy(fs.superblock.inodes_count), fs.superblock.blocks_count_lo | ((uint64_t)fs.superblock.blocks_count_hi << 32));

        log::log$("ext4: block size: {}", 1024 << fs.superblock.log_block_size);
        log::log$("ext4: inode size: {}", core::copy(fs.superblock.inode_size));
        log::log$("ext4: blocks per group: {}", core::copy(fs.superblock.blocks_per_group));
        log::log$("ext4: inodes per group: {}", core::copy(fs.superblock.inodes_per_group));
        log::log$("ext4: first inode: {}", core::copy(fs.superblock.first_ino));
        log::log$("ext4: feature compat: {}", core::copy(fs.superblock.feature_compat));
        log::log$("ext4: feature incompat: {}", core::copy(fs.superblock.feature_incompat));
        log::log$("ext4: feature ro compat: {}", core::copy(fs.superblock.feature_ro_compat));

        fs.disk_asset = Wingos::Space::self().allocate_physical_memory(math::alignUp<size_t>((1024 << fs.superblock.log_block_size), (size_t)4096));
        fs.mapped_disk_asset = Wingos::Space::self().map_memory(fs.disk_asset, ASSET_MAPPING_FLAG_READ | ASSET_MAPPING_FLAG_WRITE);

        fs.block_desc_size = 4;

        if(fs.superblock.rev_level >= 1 && fs.superblock.feature_incompat & EXT4_FEAT_64BIT)
        {
            fs.block_desc_size = fs.superblock.desc_size;
        }
        else
        {
            fs.block_desc_size = 32;
        }

        fs.bgd_table_start_block = fs.superblock.first_data_block + 1;
        if (fs.superblock.feature_incompat & EXT4_FEAT_META_BG)
        {
            fs.bgd_table_start_block += fs.superblock.first_meta_bg * (fs.superblock.inodes_per_group * fs.superblock.inode_size + fs.block_desc_size - 1) / (1024 << fs.superblock.log_block_size);
        }

        

        auto v = try$(fs.read_inode(2));

        log::log$("ext4: root inode has {} blocks", core::copy(v.inode.blocks_lo));
        log::log$("ext4: root inode size: {}", (uint64_t)v.inode.size_lo);
        log::log$("ext4: root inode first block pointer: {}", core::copy(v.inode.block[0]));
        log::log$("ext4: root inode type: {}", (uint16_t)v.inode.file_type);


        fs.dump_subdir(v, 0);

        return fs;
    }

    core::Result<Ext4InodeRef> get_subdir(Ext4InodeRef const& dir_inode, core::Str const & name);



    
};
