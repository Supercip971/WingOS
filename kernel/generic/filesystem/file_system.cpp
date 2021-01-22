#include "file_system.h"
#include "filesystem/echfs.h"
#include "filesystem/ext2fs.h"
#include <logging.h>
file_system::file_system()
{
}

main_fs_system sys;

main_fs_system *main_fs_system::the()
{
    return &sys;
}
file_system *main_fs_system::from_partition(uint64_t partition_id)
{
    return file_systems[partition_id];
}
file_system *main_fs_system::main_fs()
{
    file_system *fs = from_partition(0);
    if (fs == nullptr)
    {
        log("main fs", LOG_INFO) << "ow ow";
    }
    return fs;
}
void main_fs_system::init_file_system()
{
    for (int i = 0; i < 4; i++)
    {
        file_systems[i] = nullptr;
    }
    log("main fs", LOG_DEBUG) << "loading main fs";
    MBR_partition *mbr = new MBR_partition();
    mbr->init();
    partition_system = (base_partition *)mbr;
    for (int i = 0; i <= mbr->get_parition_count(); i++)
    {
        log("main fs", LOG_INFO) << "checking partition entry" << i;
        if (echfs::is_valid_echfs_entry(mbr->get_partition_start(i)))
        {
            log("main fs", LOG_INFO) << "entry " << i << "is an echfs entry";
            echfs *ech_file_sys = new echfs();
            file_systems[i] = dynamic_cast<file_system *>(ech_file_sys);
            file_systems[i]->init(mbr->get_partition_start(i), mbr->get_partition_length(i));
        }
        else if (ext2fs::is_valid_ext2fs_entry(mbr->get_partition_start(i)))
        {
            log("main fs", LOG_INFO) << "entry " << i << "is an ext2fs entry";
            ext2fs *ext_file_sys = new ext2fs();
            file_systems[i] = dynamic_cast<file_system *>(ext_file_sys);
            file_systems[i]->init(mbr->get_partition_start(i), mbr->get_partition_length(i));
        }
        else
        {

            log("main fs", LOG_WARNING) << "entry " << i << "is not a valid entry";
        }
    }
}
