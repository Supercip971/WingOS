#include "ext2fs.h"

#include <arch.h>
#include <device/ata_driver.h>
#include <filesystem/echfs.h>
#include <io_device.h>
#include <kernel.h>
#include <logging.h>
#include <physical.h>
#include <utility.h>
#include <utils/liballoc.h>
#include <utils/lock.h>
#include <utils/smart_ptr.h>

ext2fs::ext2fs()
    : file_system()
{
}

ext2fs_inode::ext2fs_inode(ext2fs_inode &other)
{
    this->id = other.id;
    memcpy(&this->strct, &other.strct, sizeof(ext2fs_inode_structure));
}
ext2fs_inode::ext2fs_inode(const ext2fs_inode &other)
{
    this->id = other.id;
    memcpy(&this->strct, &other.strct, sizeof(ext2fs_inode_structure));
}
void ext2fs::read_block(uint64_t block_id, uint8_t *buffer)
{

    get_io_device(0)->read(buffer, block_size / 512, (offset + (block_id * block_size)) / 512);
}
void ext2fs::read_blocks(uint64_t block_id, uint64_t length, uint8_t *buffer)
{
    for (uint64_t i = 0; i < length; i++)
    {
        get_io_device(0)->read((buffer + ((block_size * i))), ((block_size) / 512), (offset + ((block_id + i) * block_size)) / 512);
    }
}
ext2fs_block_group_descriptor ext2fs::read_group_from_inode(uint64_t inode)
{
    uint64_t current_block_group = (inode - 1) / super_block.inode_count_per_group;
    uint64_t block_group_start = block_group_descriptor_table * block_size;
    auto group = utils::make_unique<ext2fs_block_group_descriptor>();
    uint64_t group_offset = block_group_start + ((sizeof(ext2fs_block_group_descriptor) * current_block_group));
    get_io_device(0)->read_unaligned((uint8_t *)(group.get_raw()), sizeof(ext2fs_block_group_descriptor), offset + group_offset);
    ext2fs_block_group_descriptor rgroup = group.get();
    return rgroup;
}

void ext2fs::write_group_from_inode(uint64_t inode, ext2fs_block_group_descriptor group)
{
    uint64_t current_block_group = (inode - 1) / super_block.inode_count_per_group;
    uint64_t block_group_start = block_group_descriptor_table * block_size;

    uint64_t group_offset = block_group_start + ((sizeof(ext2fs_block_group_descriptor) * current_block_group));
    get_io_device(0)->write_unaligned((uint8_t *)(&group), sizeof(ext2fs_block_group_descriptor), offset + group_offset);
}
ext2fs_block_group_descriptor ext2fs::read_group_from_group_id(uint64_t gid)
{
    uint64_t current_block_group = gid;
    uint64_t block_group_start = block_group_descriptor_table * block_size;

    auto group = utils::make_unique<ext2fs_block_group_descriptor>();
    uint64_t group_offset = block_group_start + ((sizeof(ext2fs_block_group_descriptor) * current_block_group));
    get_io_device(0)->read_unaligned((uint8_t *)(group.get_raw()), sizeof(ext2fs_block_group_descriptor), offset + group_offset);
    ext2fs_block_group_descriptor rgroup = group.get();
    return rgroup;
}
void ext2fs::write_group_from_group_id(uint64_t gid, ext2fs_block_group_descriptor group)
{
    uint64_t current_block_group = gid;
    uint64_t block_group_start = block_group_descriptor_table * block_size;

    uint64_t group_offset = block_group_start + ((sizeof(ext2fs_block_group_descriptor) * current_block_group));
    get_io_device(0)->write_unaligned((uint8_t *)(&group), sizeof(ext2fs_block_group_descriptor), offset + group_offset);
}

bool ext2fs::inode_read(void *buffer, uint64_t cursor, uint64_t count, ext2fs_inode parent)
{
    for (uint64_t readed = 0; readed < count;)
    {
        uint64_t current_block = 0;
        uint64_t block_offset = 0;
        current_block = (cursor + readed) / block_size;
        if (current_block > parent.strct.block_count)
        {
            log("ext2fs", LOG_ERROR, "trying to read out of bound of block at {} > {}", current_block, parent.strct.block_count);
            break;
        }
        uint64_t chunk_size_to_read = count - readed;

        block_offset = (cursor + readed) % block_size;
        if (chunk_size_to_read > block_size - block_offset)
        {
            chunk_size_to_read = block_size - block_offset;
        }
        auto block = get_inode_block_map(parent, current_block);

        get_io_device(0)->read_unaligned((uint8_t *)buffer + readed, chunk_size_to_read, (offset + (block * block_size)) + block_offset);
        readed += chunk_size_to_read;
    }
    return true;
}
bool ext2fs::inode_write(const void *buffer, uint64_t cursor, uint64_t count, ext2fs_inode parent)
{
    for (uint64_t writed = 0; writed < count;)
    {
        uint64_t current_block = 0;
        uint64_t block_offset = 0;
        current_block = (cursor + writed) / block_size;
        if (current_block > parent.strct.block_count)
        {
            log("ext2fs", LOG_ERROR, "trying to write out of bound of block at {} > {}", current_block, parent.strct.block_count);
            break;
        }
        uint64_t chunk_size_to_write = count - writed;

        block_offset = (cursor + writed) % block_size;
        if (chunk_size_to_write > block_size - block_offset)
        { // if the block is too big
            chunk_size_to_write = block_size - block_offset;
        }
        auto block = get_inode_block_map(parent, current_block);
        //  log("ext2fs", LOG_INFO) << "writing inode " << chunk_size_to_write << "offset" << block << " super block size " << block_size << " with block " << current_block << " writed " << writed << " count " << count;
        get_io_device(0)->write_unaligned((uint8_t *)buffer + writed, chunk_size_to_write, (offset + (block * block_size)) + block_offset);
        writed += chunk_size_to_write;
    }
    return true;
}
uint64_t ext2fs::get_inode_block_map(ext2fs_inode inode_struct, uint64_t block_id)
{
    if (inode_struct.strct.flag & 0x80000)
    {
        log("ext2fs", LOG_ERROR, "not supported file flag EXTENT FLAG");
    };
    if (block_id > inode_struct.strct.block_count)
    {
        log("ext2fs", LOG_WARNING, "block: {} out of bound", block_id);
        return 0;
    }
    uint64_t r = 0;
    uint64_t entry_per_blkc = block_size / sizeof(uint32_t);

    if (block_id < 12)
    {
        r = inode_struct.strct.block_ptr[block_id];
    }
    // indirect block start after block 13
    else if ((block_id - 12) < entry_per_blkc)
    { // indirect block
        uint32_t nid = block_id - 12;
        get_io_device(0)->read_unaligned((uint8_t *)(&r), sizeof(uint32_t), offset + inode_struct.strct.singly_indirect_block_ptr * block_size + (nid * sizeof(uint32_t)));
    }
    // double indirect block start after (entry per block)
    else if (((block_id - 12) / entry_per_blkc) < entry_per_blkc)
    { // double indirect

        uint32_t nid = (block_id - (12 + entry_per_blkc));
        uint32_t block_idx = nid / entry_per_blkc;
        uint32_t indirect_block_id = 0;
        uint32_t sub_entry = nid % entry_per_blkc;
        get_io_device(0)->read_unaligned((uint8_t *)(&indirect_block_id), sizeof(uint32_t), offset + inode_struct.strct.doubly_indirect_block_ptr * block_size + (block_idx * sizeof(uint32_t)));

        get_io_device(0)->read_unaligned(
            (uint8_t *)(&r),
            sizeof(uint32_t),
            offset + indirect_block_id * block_size + ((sub_entry) * sizeof(uint32_t)));
    }
    else
    {
        log("ext2fs", LOG_WARNING, "no support for triple indirect block: {}", block_id);
        while (true)
        {
        }
    }
    return r;
}
void ext2fs::add_inode_block_map(ext2fs_inode &inode_struct, uint32_t block_addr)
{
    if (inode_struct.strct.flag & 0x80000)
    {
        log("ext2fs", LOG_ERROR, "not supported file flag EXTENT FLAG");
        ;
    };
    uint64_t entry_per_blkc = block_size / sizeof(uint32_t);
    uint64_t block_id = inode_struct.strct.block_count - 1;
    inode_struct.strct.block_count++;

    log("ext2fs", LOG_INFO, "adding block {} at {}", inode_struct.strct.block_count - 1, block_addr);
    if (block_id < 12)
    {
        inode_struct.strct.block_ptr[block_id] = block_addr;
        write_inode(inode_struct);
        return;
    }
    // indirect block start after block 13
    else if ((block_id - 12) < entry_per_blkc)
    { // indirect block
        if (block_id - 1 < 12)
        { // create for single indirection
            inode_struct.strct.singly_indirect_block_ptr = alloc_block_for_inode(inode_struct);
        }
        uint32_t nid = block_id - 12;
        get_io_device(0)->write_unaligned((uint8_t *)(&block_addr), sizeof(uint32_t), offset + inode_struct.strct.singly_indirect_block_ptr * block_size + (nid * sizeof(uint32_t)));
        write_inode(inode_struct);
        return;
    }
    // double indirect block start after (entry per block)
    else if (((block_id - 12) / entry_per_blkc) < entry_per_blkc)
    { // double indirect
        if ((block_id - 1) < entry_per_blkc)
        { // create for single indirection
            inode_struct.strct.doubly_indirect_block_ptr = alloc_block_for_inode(inode_struct);
        }
        uint32_t nid = (block_id - (12 + entry_per_blkc));
        uint32_t block_idx = nid / entry_per_blkc;
        uint32_t indirect_block_id = 0;
        uint32_t sub_entry = nid % entry_per_blkc;
        get_io_device(0)->read_unaligned((uint8_t *)(&indirect_block_id), sizeof(uint32_t), offset + inode_struct.strct.doubly_indirect_block_ptr * block_size + (block_idx * sizeof(uint32_t)));
        if (indirect_block_id == 0)
        {

            indirect_block_id = alloc_block_for_inode(inode_struct);
            get_io_device(0)->write_unaligned((uint8_t *)(&indirect_block_id), sizeof(uint32_t), offset + inode_struct.strct.doubly_indirect_block_ptr * block_size + (block_idx * sizeof(uint32_t)));
        }
        get_io_device(0)->write_unaligned(
            (uint8_t *)(&block_addr),
            sizeof(uint32_t),
            offset + indirect_block_id * block_size + ((sub_entry) * sizeof(uint32_t)));
        write_inode(inode_struct);
        return;
    }
    else
    {
        log("ext2fs", LOG_WARNING, "no support for writing triple indirect block: {}", block_id);
        while (true)
        {
        }
    }
}
uint32_t *ext2fs::create_inode_block_map(ext2fs_inode inode_struct)
{
    if (inode_struct.strct.flag & 0x80000)
    {
        log("ext2fs", LOG_ERROR, "not supported file flag EXTENT FLAG");
    };
    uint64_t entry_per_blkc = block_size / sizeof(uint32_t);
    uint32_t *data = (uint32_t *)malloc(((inode_struct.strct.block_count + 2) * sizeof(uint32_t))); // why does clang say that this is garbage ? O.o
    for (uint32_t block_id = 0; block_id < inode_struct.strct.block_count; block_id++)
    {
        if (block_id < 12)
        {
            data[block_id] = inode_struct.strct.block_ptr[block_id];
        }
        // indirect block start after block 13
        else if ((block_id - 12) < entry_per_blkc)
        { // indirect block
            uint32_t nid = block_id - 12;

            get_io_device(0)->read_unaligned((uint8_t *)(data + block_id), sizeof(uint32_t), offset + inode_struct.strct.singly_indirect_block_ptr * block_size + (nid * sizeof(uint32_t)));
        }
        // double indirect block start after (entry per block)
        else if (((block_id - 12) / entry_per_blkc) < entry_per_blkc)
        { // double indirect

            uint32_t nid = (block_id - (12 + entry_per_blkc));
            uint32_t block_idx = nid / entry_per_blkc;
            uint32_t indirect_block_id = 0;
            get_io_device(0)->read_unaligned((uint8_t *)(&indirect_block_id), sizeof(uint32_t), offset + inode_struct.strct.doubly_indirect_block_ptr * block_size + (block_idx * sizeof(uint32_t)));

            for (uint32_t sub_entry = 0; sub_entry < entry_per_blkc; sub_entry++)
            {
                if (block_id + sub_entry > inode_struct.strct.block_count)
                {
                    log("ext2fs", LOG_INFO, "double indirect dend {}", block_id);
                    return data;
                }
                get_io_device(0)->read_unaligned(
                    (uint8_t *)(data + block_id + sub_entry),
                    sizeof(uint32_t),
                    offset + indirect_block_id * block_size + (sub_entry * sizeof(uint32_t)));
            }
            block_id += entry_per_blkc - 1;
        }
        else
        {
            log("ext2fs", LOG_WARNING, "no support for triple indirect block {}", block_id);
            while (true)
            {
            }
        }
    }
    return data;
}
ext2fs_inode ext2fs::get_inode(uint64_t inode)
{
    ext2fs_inode rret;
    auto ret = utils::make_unique<ext2fs_inode_structure>();
    uint64_t current_block_group = (inode - 1) / super_block.inode_count_per_group;
    uint64_t inside_block_group = (inode - 1) % super_block.inode_count_per_group;
    uint64_t block_group_start = block_group_descriptor_table * block_size;

    uint64_t inode_size = super_block.inode_size;

    auto group = utils::make_unique<ext2fs_block_group_descriptor>();
    uint64_t group_offset = block_group_start + ((sizeof(ext2fs_block_group_descriptor) * current_block_group));
    get_io_device(0)->read_unaligned((uint8_t *)(group.get_raw()), sizeof(ext2fs_block_group_descriptor), offset + group_offset);

    uint64_t inode_offset = (group->inode_table * block_size) + (inode_size * inside_block_group);
    get_io_device(0)->read_unaligned((uint8_t *)(ret.get_raw()), sizeof(ext2fs_inode_structure), offset + inode_offset);

    rret.strct = ret.get();
    rret.id = inode;
    return rret;
}
bool ext2fs::write_inode(ext2fs_inode inode)
{
    uint64_t current_block_group = (inode.id - 1) / super_block.inode_count_per_group;
    uint64_t inside_block_group = (inode.id - 1) % super_block.inode_count_per_group;
    uint64_t block_group_start = block_group_descriptor_table * block_size;

    uint64_t inode_size = super_block.inode_size;

    auto group = utils::make_unique<ext2fs_block_group_descriptor>();
    uint64_t group_offset = block_group_start + ((sizeof(ext2fs_block_group_descriptor) * current_block_group));
    get_io_device(0)->read_unaligned((uint8_t *)(group.get_raw()), sizeof(ext2fs_block_group_descriptor), offset + group_offset);

    uint64_t inode_offset = (group->inode_table * block_size) + (inode_size * inside_block_group);
    get_io_device(0)->write_unaligned((uint8_t *)(&inode.strct), sizeof(ext2fs_inode_structure), offset + inode_offset);
    return true;
}
bool ext2fs::is_valid_ext2fs_entry(uint64_t start_sector)
{

    auto temp_buffer = utils::unique_ptr<uint8_t>(new uint8_t[1024]);

    get_io_device(0)->read(temp_buffer.get_raw(), 2, (start_sector + 1024) / 512);

    ext2fs_superblock hdr = *temp_buffer.template as<ext2fs_superblock *>();
    if (hdr.signature != EXT2FS_SIGNATURE)
    {
        return false;
    }
    return true;
}
// oof that's so bad code :^(
void ext2fs::print_ext2_feature()
{

    // OPTIONAL FEATURE

    log("ext2", LOG_INFO, "ext2fs optional features: {}", super_block.optional_features);

    if (super_block.optional_features & PREALOC_BLOCK)
    {
        log("ext2", LOG_INFO, " - optional feature : PREALOC_BLOCK");
    }
    if (super_block.optional_features & AFS_INODE)
    {
        log("ext2", LOG_INFO, " - optional feature : AFS_INODE ");
    }
    if (super_block.optional_features & JOURNAL_SUPPORT)
    {
        log("ext2", LOG_INFO, " - optional feature : JOURNAL_SUPPORT ");
    }
    if (super_block.optional_features & EXTENDED_INODE_ATTRIB)
    {
        log("ext2", LOG_INFO, " - optional feature : EXTENDED_INODE_ATTRIB ");
    }
    if (super_block.optional_features & RESIZABLE_FS)
    {
        log("ext2", LOG_INFO, " - optional feature : RESIZABLE_FS ");
    }
    if (super_block.optional_features & INDEXED_DIRECTORY)
    {
        log("ext2", LOG_INFO, " - optional feature : INDEXED_DIRECTORY ");
    }

    // READ ONLY FEATURES
    log("ext2", LOG_INFO, "ext2fs read only features: {}", super_block.read_only_features);

    if (super_block.read_only_features & SPARSE_DESC_TABLE)
    {
        log("ext2", LOG_INFO, " - read only feature : SPARSE_DESC_TABLE ");
    }
    if (super_block.read_only_features & LONG_MODE_FILE_SIZE)
    {
        log("ext2", LOG_INFO, " - read only feature : LONG_MODE_FILE_SIZE ");
    }
    if (super_block.read_only_features & BINARY_TREE_DIRECTORY_CONTENT)
    {
        log("ext2", LOG_INFO, " - read only feature : BINARY_TREE_DIRECTORY_CONTENT ");
    }

    // NEEDED FEATURES
    log("ext2", LOG_INFO, "ext2fs needed features: {}", super_block.required_features);

    if (super_block.required_features & COMPRESSION)
    {
        log("ext2", LOG_INFO, " - needed feature : COMPRESSION ");
    }
    if (super_block.required_features & DIRECTORY_TYPE_FIELD)
    {
        log("ext2", LOG_INFO, " - needed feature : DIRECTORY_TYPE_FIELD ");
    }
    if (super_block.required_features & JOURNAL_REPLAY_NEEDED)
    {
        log("ext2", LOG_INFO, " - needed feature : JOURNAL_REPLAY_NEEDED ");
    }
    if (super_block.required_features & JOURNAL_DEVICE_NEEDED)
    {
        log("ext2", LOG_INFO, " - needed feature : JOURNAL_DEVICE_NEEDED ");
    }
}
void ext2fs::list_sub_directory(ext2fs_inode inode, int offset)
{
    auto dir = utils::make_unique<ext2fs_directory>();
    int v = 0;
    for (uint32_t i = 0; i < inode.strct.lower_size;)
    {

        inode_read(dir.get_raw(), i, sizeof(ext2fs_directory), inode);
        for (int i = 0; i < offset; i++)
        {
            printf("\t");
        }
        printf("%x %s %x \n", v, dir->name, dir->type);
        if (dir->type == ext2fs_directory_type::DIRRECTORY_DIR && v >= 3)
        {
            auto vi = get_inode(dir->inode_dir);
            list_sub_directory(vi, offset + 1);
        }
        i += dir->directory_size;
        v++;
    }
}
ext2fs_inode ext2fs::find_subdir(ext2fs_inode inode_struct, const char *name)
{
    auto dir = utils::make_unique<ext2fs_directory>();

    for (uint32_t i = 0; i < inode_struct.strct.lower_size;)
    {
        inode_read(dir.get_raw(), i, sizeof(ext2fs_directory), inode_struct);
        if (strncmp(dir->name, name, dir->directory_name_length) == 0)
        {
            auto vi = get_inode(dir->inode_dir);
            return vi;
        }
        i += dir->directory_size;
    }
    return ext2fs_inode(); // not founded
}
ext2fs_inode ext2fs::get_file(const char *path)
{
    ext2fs_inode f = get_inode(2);
    uint32_t last_path_cur = 0;
    uint32_t searching_file_cur = 0;
    bool end_of_search = false;

    auto searching_file = utils::unique_ptr<char>(new char[255]);
    while (true)
    {
        searching_file_cur = 0;
        for (; path[last_path_cur] != '/' && last_path_cur <= strlen(path); last_path_cur++)
        {
            searching_file[searching_file_cur] = path[last_path_cur];
            searching_file_cur++;
        }
        last_path_cur++;
        if (last_path_cur >= strlen(path))
        {
            end_of_search = true;
        }
        ext2fs_inode v = find_subdir(f, searching_file.get_raw());

        if (!v.is_valid())
        {

            return ext2fs_inode();
        }
        else
        {
            f = v;
            if (end_of_search)
            {
                return v;
            }
        }
    }
}
uint8_t *ext2fs::ext_read_file(const char *path)
{
    ext_lock.lock();
    log("ext2fs", LOG_INFO, "reading {}", path);
    ext2fs_inode f = get_file(path);
    if (!f.is_valid())
    {
        log("ext2fs", LOG_WARNING, "can't find file {} for {}", path, __PRETTY_FUNCTION__);
        ext_lock.unlock();
        return 0;
    }
    auto dat = utils::unique_ptr<uint8_t>(new uint8_t[f.strct.lower_size]);

    inode_read(dat.get_raw(), 0, f.strct.lower_size, f);

    ext_lock.unlock();
    return dat.release();
}
bool ext2fs::exist(const char *path)
{

    ext_lock.lock();
    ext2fs_inode f = get_file(path);
    ext_lock.unlock();
    return f.is_valid();
}
uint64_t ext2fs::get_file_length(const char *path)
{
    ext_lock.lock();
    log("ext2fs", LOG_INFO, "getting file length {}", path);
    ext2fs_inode f = get_file(path);
    if (!f.is_valid())
    {
        ext_lock.unlock();
        return (uint64_t)-1;
    }
    uint64_t size = f.strct.lower_size;
    ext_lock.unlock();
    log("ext2fs", LOG_INFO, "getting file length {} = {}", path, size);
    return size;
}
void ext2fs::init(uint64_t start_sector, uint64_t sector_count)
{
    offset = start_sector;
    log("ext2", LOG_DEBUG, "loading ext2fs at {}", start_sector);

    auto temp_buffer = utils::unique_ptr<uint8_t>(new uint8_t[1024]);
    get_io_device(0)->read_unaligned(temp_buffer.get_raw(), 1024, (start_sector + 1024));
    super_block = *temp_buffer.template as<ext2fs_superblock *>();
    block_size = ((uint64_t)(1024) << super_block.sblock_size);
    if (block_size == 1024)
    {
        block_group_descriptor_table = 2;
    }
    else
    {
        block_group_descriptor_table = 1;
    }
    log("ext2", LOG_INFO, "ext2fs major version: {}", super_block.version);
    log("ext2", LOG_INFO, "ext2fs minor version: {}", super_block.minor_version);
    log("ext2", LOG_INFO, "ext2fs block size: {}", block_size);
    log("ext2", LOG_INFO, "ext2fs inode per group: {}", super_block.inode_count_per_group);
    log("ext2", LOG_INFO, "ext2fs inode size: {}", super_block.inode_size);
    print_ext2_feature();
    root_inode = get_inode(2);
    auto dir = utils::make_unique<ext2fs_directory>();
    inode_read(dir.get_raw(), 0, sizeof(ext2fs_directory), root_inode);
    log("ext2", LOG_INFO, "root inode dir: {}", dir->name);
    log("ext2", LOG_INFO, "root inode name length: {}", dir->directory_name_length);
    log("ext2", LOG_INFO, "root inode block subdir: {}", dir->inode_dir);
    log("ext2", LOG_INFO, "root inode size: {}", root_inode.strct.lower_size);
    list_sub_directory(root_inode, 0);
    get_file("initfs/test_directory/test_another.txt");
}

uint64_t ext2fs::read_file(const char *path, uint64_t at, uint64_t size, uint8_t *buffer)
{

    ext_lock.lock();
    auto f = get_file(path);
    if (!f.is_valid())
    {
        log("ext2fs", LOG_WARNING, "can't find file {} for {}", path, __PRETTY_FUNCTION__);

        ext_lock.unlock();
        return 0;
    }
    uint64_t readed_size = size;
    if (f.strct.lower_size < at)
    {
        log("ext2fs", LOG_WARNING, "cursor of {} is out of file bound: {} <= {}", path, at, f.strct.lower_size);

        ext_lock.unlock();
        return 0;
    }
    if (f.strct.lower_size < at + size)
    {
        readed_size = f.strct.lower_size - at;
    }
    auto buffer_copy = utils::unique_ptr<uint8_t>(new uint8_t[readed_size + 12]);

    inode_read(buffer_copy.get_raw(), at, readed_size, f);
    memcpy(buffer, buffer_copy.get_raw(), size);
    ext_lock.unlock();
    return readed_size;
}
uint64_t ext2fs::write_file(const char *path, uint64_t at, uint64_t size, const uint8_t *buffer)
{

    ext_lock.lock();
    auto f = get_file(path);
    if (!f.is_valid())
    {
        log("ext2fs", LOG_WARNING, "can't find file {} for {}", path, __PRETTY_FUNCTION__);

        ext_lock.unlock();
        return 0;
    }
    uint64_t write_size = size + at;

    if (f.strct.block_count * block_size < (write_size))
    {
        /*   resize_file(f, write_size); */
        log("ext2fs", LOG_WARNING, "can't resize file {}", path);
        ext_lock.unlock();
        return 0;
    }
    else if (f.strct.lower_size < write_size)
    {
        f.strct.lower_size = write_size;
    }
    write_inode(f);
    inode_write(buffer, at, size, f);

    ext_lock.unlock();
    return size;
}
uint64_t ext2fs::alloc_block_for_inode(ext2fs_inode &inode)
{
    auto group = read_group_from_inode(inode.id);
    size_t gid = get_group_from_inode(inode.id);
    if (group.free_block < 1)
    {
        while (group.free_block < 1)
        {
            group = read_group_from_group_id(gid);
            log("ext2fs", LOG_WARNING, "group haven't free blocks: {}", gid);
            gid++;
        }
    }
    log("ext2fs", LOG_INFO, "block group size: {}", super_block.block_count_per_group * block_size);

    uint8_t *bitmap = (uint8_t *)malloc(block_size);
    memzero(bitmap, block_size);
    get_io_device(0)->read_unaligned(bitmap, block_size, group.block_bitmap * block_size + offset);
    uint64_t start_block = 0;
    bool founded = false;
    for (size_t i = 0; i < block_size; i++)
    { // find free block
        if (!get_bit(bitmap, i))
        {
            start_block = i;
            founded = true;
            break;
        }
    }
    if (!founded)
    {
        log("ext2fs", LOG_ERROR, "group haven't free blocks founded {}", gid);
        return false;
    }
    // set founded block as used
    set_bit(bitmap, start_block, 1);
    group.free_block -= 1;

    write_group_from_group_id(gid, group);

    get_io_device(0)->write_unaligned(bitmap, block_size, group.block_bitmap * block_size + offset);
    uint64_t block_offset = (super_block.block_count_per_group * gid) + (start_block);
    free(bitmap);
    clear_block(block_offset);

    return block_offset;
}
void ext2fs::clear_block(size_t block_addr)
{
    uint8_t *block_data = new uint8_t[block_size];
    memzero(block_data, block_size);
    get_io_device(0)->write_unaligned(block_data, block_size, block_addr * block_size + offset);
    delete[] block_data;
}
void ext2fs::resize_file(ext2fs_inode &inode, uint64_t new_size)
{
    uint64_t block_diff = (new_size - inode.strct.lower_size) / block_size;
    log("ext2fs", LOG_INFO, "resizing inode: {} from: {} to: {} .. new block count: {}", inode.id, inode.strct.lower_size, new_size, block_diff);
    block_diff++;
    for (uint64_t i = 0; i < block_diff; i++)
    {
        auto res = alloc_block_for_inode(inode);
        log("ext2fs", LOG_INFO, "alloc block ret {}", res);
        add_inode_block_map(inode, res);
        log("ext2fs", LOG_INFO, "alloc block rret {}", get_inode_block_map(inode, inode.strct.block_count - 1));
    }
    inode.strct.block_count += block_diff;
    write_inode(inode);
}
constexpr uint64_t ext2fs::get_group_from_inode(uint64_t inode)
{
    return (inode - 1) / super_block.inode_count_per_group;
}
constexpr uint64_t ext2fs::get_local_group_inode_from_inode(uint64_t inode)
{
    return (inode - 1) % super_block.inode_count_per_group;
}
