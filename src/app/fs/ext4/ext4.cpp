#include "ext4.hpp"

#include "libcore/str_writer.hpp"
core::Result<void *> Ext4Filesystem::read_block_tmp(size_t block_num)
{
    try$(disk.read(disk_asset, start_lba + (block_num * (1024 << superblock.log_block_size)) / disk_block_size, (1024 << superblock.log_block_size)));


    return mapped_disk_asset.ptr();
}

core::Result<void> Ext4Filesystem::write_block_tmp(size_t block_num, void *data)
{
    if (mapped_disk_asset.ptr() != data)
    {
        memcpy(mapped_disk_asset.ptr(), data, (1024 << superblock.log_block_size));
    }
    try$(disk.write(disk_asset, start_lba + (block_num * (1024 << superblock.log_block_size)) / disk_block_size, (1024 << superblock.log_block_size)));
    return {};
}

core::Result<void> Ext4Filesystem::write_blockgroup_descriptor(BlockGroupId bg_id, Ext4BlockGroupDescriptor const &bgd)
{

    size_t block_group_entry_size = this->block_desc_size;
    size_t local_block_offset = (bg_id * block_group_entry_size) % block_size();
    size_t disk_sector = (bg_id * block_group_entry_size) / block_size();
    auto bgd_block_res = try$(read_block_tmp(bgd_table_start_block + disk_sector));
    auto bgd_ptr = (uint8_t *)bgd_block_res;

    bgd_ptr += local_block_offset;
    *(Ext4BlockGroupDescriptor *)bgd_ptr = bgd;

    try$(write_block_tmp(bgd_table_start_block + disk_sector, bgd_block_res));
    return {};
}

core::Result<Ext4BlockGroupDescriptor> Ext4Filesystem::read_blockgroup_descriptor(BlockGroupId bg_id)
{

    size_t block_group_entry_size = this->block_desc_size;
    size_t local_block_offset = (bg_id * block_group_entry_size) % block_size();
    size_t disk_sector = (bg_id * block_group_entry_size) / block_size();
    auto bgd_block_res = try$(read_block_tmp(bgd_table_start_block + disk_sector));
    auto bgd_ptr = (uint8_t *)bgd_block_res;

    bgd_ptr += local_block_offset;
    return *(Ext4BlockGroupDescriptor *)bgd_ptr;
}

core::Result<Ext4InodeRef> Ext4Filesystem::read_inode(InodeId inode)
{
    auto bg_id = blockgroup_from_inode(inode);
    auto bgd_res = try$(read_blockgroup_descriptor(bg_id));

    size_t inode_table_block = bgd_res.inode_table;
    size_t inode_index = blockgroup_inode_index(inode);
    size_t local_block_offset = (inode_index * superblock.inode_size) % block_size();
    size_t disk_sector = (inode_index * superblock.inode_size) / block_size();

    auto inode_block_res = try$(read_block_tmp(inode_table_block + disk_sector));
    auto inode_ptr = (uint8_t *)inode_block_res;
    inode_ptr += local_block_offset;


    Ext4InodeRef inode_ref;
    inode_ref.inode_id = inode;
    inode_ref.inode = *(Ext4Inode *)inode_ptr;
    return inode_ref;
}
core::Result<uint64_t> Ext4Filesystem::inode_find_block(Ext4InodeRef const &inode, size_t block)
{

    if (block > inode.inode.blocks_lo)
    {
        return "block out of range";
    }

    if (block < 12)
    {
        return inode.inode.block[block];
    }
    else if (block < 12 + (block_size() / sizeof(uint32_t)))
    {
        // single indirect
        size_t indirect_block_index = block - 12;
        auto indirect_block_res = try$(read_block_tmp( inode.inode.block[12]));
        auto block_entries = (uint32_t *)indirect_block_res;

        return block_entries[indirect_block_index];
    }
    else if (block < 12 + (block_size() / sizeof(uint32_t)) + (block_size() / sizeof(uint32_t)) * (block_size() / sizeof(uint32_t)))
    {
        // double indirect
        size_t double_indirect_index = block - 12 - (block_size() / sizeof(uint32_t));
        size_t first_level_index = double_indirect_index / (block_size() / sizeof(uint32_t));
        size_t second_level_index = double_indirect_index % (block_size() / sizeof(uint32_t));

        auto first_level_block_res = try$(read_block_tmp( inode.inode.block[13]));
        auto first_level_entries = (uint32_t *)first_level_block_res;

        auto second_level_block_res = try$(read_block_tmp( first_level_entries[first_level_index]));
        auto second_level_entries = (uint8_t *)second_level_block_res;

        return second_level_entries[second_level_index];
    }
    else
    {
        return "triple indirect blocks not supported yet";
    }
}

core::Result<size_t> Ext4Filesystem::inode_read(Ext4InodeRef const &inode, Wingos::MemoryAsset &out, size_t off, size_t len)
{

    log::log$("ext4: inode_read inode {} off {} len {}", core::copy(inode.inode_id), off, len);
    log::log$("ext4: inode size_lo {}", core::copy(inode.inode.size_lo));
    len = core::max(core::min<long>(len, (long)(inode.inode.size_lo) - (long)off), 0);
    size_t block_size_ = block_size();
    size_t start_block = off / block_size_;
    size_t end_block = (off + len) / block_size_;
    size_t block_offset = off % block_size_;

    size_t bytes_read = 0;

    auto vfile = Wingos::Space::self().map_memory(out, ASSET_MAPPING_FLAG_READ | ASSET_MAPPING_FLAG_WRITE);

    // first_block
    if (block_offset != 0)
    {
        auto block_data_res = try$(inode_read_tmp(inode, start_block));
        //                v end here
        // ##------------- ############# <- Another block
        //      | len      #      |    #
        size_t to_read = core::min(len, block_size_ - block_offset);
        memcpy((uint8_t *)vfile.ptr() + bytes_read, (uint8_t *)block_data_res + block_offset, to_read);
        bytes_read += to_read;
        start_block += 1;
    }

    // middle blocks
    for (size_t b = start_block; b < end_block; b++)
    {
        auto block_data_res = try$(inode_read_tmp(inode, b));
        memcpy((uint8_t *)vfile.ptr() + bytes_read, (uint8_t *)block_data_res, block_size_);
        bytes_read += block_size_;
    }

    // last block
    size_t remaining = len - bytes_read;
    if (remaining > 0)
    {
        auto block_data_res = try$(inode_read_tmp(inode, end_block));
        memcpy((uint8_t *)vfile.ptr() + bytes_read, (uint8_t *)block_data_res, remaining);
        bytes_read += remaining;
    }

    Wingos::Space::self().release_asset(vfile);
    return len;
}
core::Result<BlockGroupId> Ext4Filesystem::find_available_group_for_alloc(BlockGroupId start_from)
{
    BlockGroupId bg_id = start_from;
    while (true)
    {
        auto bgd_res = try$(read_blockgroup_descriptor(bg_id));
        if (bgd_res.free_blocks_count > 0)
        {
            return bg_id;
        }
        bg_id++;
        if (bg_id * superblock.blocks_per_group >= superblock.blocks_count_lo)
        {
            bg_id = 0;
        }
        if (bg_id == start_from)
        {
            return "no block group with free blocks found";
        }
    }
}

core::Result<uint64_t> Ext4Filesystem::allocate_block(BlockGroupId bg_id)
{
    auto group = find_available_group_for_alloc(bg_id);
    if (group.is_error())
    {
        return group;
    }

    auto group_id = group.unwrap();
    auto bgd_res = try$(read_blockgroup_descriptor(group_id));

    size_t block_bitmap_start = bgd_res.block_bitmap;

    size_t block_bitmap_end = block_bitmap_start + (superblock.blocks_per_group / (8 * block_size()));

    for (size_t i = block_bitmap_start; i < block_bitmap_end; i++)
    {
        auto block_bitmap_data_res = try$(read_block_tmp(i));
        auto block_bitmap = (uint8_t *)block_bitmap_data_res;
        size_t block_bitmap_size = superblock.blocks_per_group / 8;
        for (size_t byte = 0; byte < block_bitmap_size; byte++)
        {
            for (size_t bit = 0; bit < 8; bit++)
            {
                if (block_bitmap[byte] == 0xFF)
                {
                    // all blocks used in this byte
                    continue;
                }
                if ((block_bitmap[byte] & (1 << bit)) == 0)
                {
                    // free block
                    block_bitmap[byte] |= (1 << bit);
                    // write back bitmap
                    try$(write_block_tmp(i, block_bitmap));
                    size_t new_block_index = (i - block_bitmap_start) * block_size() * 8 + byte * 8 + bit;
                    size_t new_block_num = group_id * superblock.blocks_per_group + new_block_index + superblock.first_data_block;

                    bgd_res.free_blocks_count -= 1;
                    // write back bgd
                    write_blockgroup_descriptor(group_id, bgd_res);
                    return new_block_num;
                }
            }
        }
    }

    return "no free blocks found in bitmap";
}

core::Result<void> Ext4Filesystem::inode_add_block(Ext4InodeRef &inode)
{

    // allocate new block
    auto bg_id = blockgroup_from_inode(inode.inode_id);
    auto block = try$(allocate_block(bg_id));

    size_t new_block_index = inode.inode.blocks_lo;
    inode.inode.blocks_lo += 1;

    if (new_block_index < 12)
    {
        inode.inode.block[new_block_index] = (uint32_t)block;
    }
    else if (new_block_index < 12 + (block_size() / sizeof(uint32_t)))
    {
        // single indirect
        size_t indirect_block_index = new_block_index - 12;
        uint32_t *indirect_block_entries = nullptr;
        if (indirect_block_index == 0)
        {
            // need to allocate indirect block
            auto indirect_block_num = try$(allocate_block(bg_id));
            inode.inode.block[12] = (uint32_t)indirect_block_num;

            indirect_block_entries = (uint32_t *)try$(read_block_tmp(indirect_block_num));
            memset(indirect_block_entries, 0, block_size());
        }
        else
        {
            indirect_block_entries = (uint32_t *)try$(inode_read_tmp(inode, inode.inode.block[12]));
        }

        indirect_block_entries[indirect_block_index] = (uint32_t)block;
        try$(write_block_tmp(inode.inode.block[12], indirect_block_entries));
    }
    else
    {
        return "triple/double indirect blocks adding not supported yet";
    }

    try$(write_inode(inode.inode_id, inode.inode));

    return {};
}
core::Result<void> Ext4Filesystem::write_inode(InodeId inode, Ext4Inode const &data)
{
    auto bg_id = blockgroup_from_inode(inode);
    auto bgd_res = try$(read_blockgroup_descriptor(bg_id));

    size_t inode_table_block = bgd_res.inode_table;
    size_t inode_index = blockgroup_inode_index(inode);
    size_t local_block_offset = (inode_index * superblock.inode_size) % block_size();
    size_t disk_sector = (inode_index * superblock.inode_size) / block_size();
    auto inode_block_res = try$(read_block_tmp(inode_table_block + disk_sector));
    auto inode_ptr = (uint8_t *)inode_block_res;
    inode_ptr += local_block_offset;
    *(Ext4Inode *)inode_ptr = data;
    try$(write_block_tmp(inode_table_block + disk_sector, inode_block_res));
    return {};
}

core::Result<void> Ext4Filesystem::dump_subdir(Ext4InodeRef const &dir_inode, int depth)
{
    size_t block_size_ = block_size();
    size_t dir_size = dir_inode.inode.size_lo;
    size_t total_blocks = (dir_size + block_size_ - 1) / block_size_;

    for (size_t b = 0; b < total_blocks; b++)
    {
        size_t bytes_processed = 0;
        while (bytes_processed < block_size_)
        {
            auto block_data_res = try$(inode_read_tmp(dir_inode, b));

            auto entry_ptr = (uint8_t *)block_data_res + bytes_processed;

            Ext4DirEntry *entry = (Ext4DirEntry *)entry_ptr;

            bytes_processed += entry->rec_len;
            if (entry->inode == 0)
            {
                continue;
            }

            core::WStr name = {};
            core::WStr raw_name = {};

            for (int i = 0; i < depth; i++)
            {
                name.append("   ");
            }

            name.append("- ");

            for (size_t i = 0; i < entry->name_len; i++)
            {
                name.put(entry->name[i]);
                raw_name.put(entry->name[i]);
            }

            log::log$("{} (inode {}) {}", name.view(), core::copy(entry->inode), core::copy(entry->file_type));

            if (entry->file_type == 2 && raw_name.view() != core::Str(".") && raw_name.view() != core::Str(".."))
            {
                dump_subdir(try$(read_inode(entry->inode)), depth + 1);
            }
        }
    }
    return {};
}

core::Result<void> Ext4Filesystem::inode_write_tmp(Ext4InodeRef &inode, size_t block, void *data)
{
    // block is offset

    while (block >= inode.inode.blocks_lo)
    {
        try$(inode_add_block(inode));
    }
    size_t block_off = block;

    auto block_num_res = try$(inode_find_block(inode, block_off));
    try$(write_block_tmp(block_num_res, data));
    return {};
}
core::Result<void *> Ext4Filesystem::inode_read_tmp(Ext4InodeRef const &inode, size_t block)
{
    // block is offset

    size_t block_off = block;

    auto block_num_res = try$(inode_find_block(inode, block_off));
    auto block_data_res = try$(read_block_tmp(block_num_res));
    return block_data_res;
}

core::Result<Ext4InodeRef> Ext4Filesystem::get_subdir(Ext4InodeRef const &dir_inode, core::Str const &name)
{
    size_t block_size_ = block_size();
    size_t dir_size = dir_inode.inode.size_lo;
    size_t total_blocks = (dir_size + block_size_ - 1) / block_size_;
    log::log$("ext4: searching for entry '{}' in directory inode {}", name.view(), dir_inode.inode_id);


    for (size_t b = 0; b < total_blocks; b++)
    {
        size_t bytes_processed = 0;
        while (bytes_processed < block_size_)
        {

            auto block_data_res = try$(inode_read_tmp(dir_inode, b));

            auto entry_ptr = (uint8_t *)block_data_res + bytes_processed;

            Ext4DirEntry *entry = (Ext4DirEntry *)entry_ptr;

            bytes_processed += entry->rec_len;
            if (entry->inode == 0)
            {
                continue;
            }

            

            core::Str ename = core::Str(entry->name, entry->name_len);

            if (ename == name)
            {
                return try$(read_inode(entry->inode));
            }
        }
    }

    return "directory entry not found";
}

//  core::Result<void> Ext4Filesystem::inode_write(InodeId inode, Wingos::MemoryAsset& out, size_t len, size_t block);
// core::Result<void> Ext4Filesystem::inode_write_tmp(InodeId inode, size_t block, void* data);
