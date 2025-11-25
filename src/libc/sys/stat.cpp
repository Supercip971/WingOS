#include "sys/stat.h"
#include <protocols/vfs/file.hpp>
#include <protocols/vfs/vfs.hpp>

int stat(const char *__restrict path, struct stat *__restrict buf)
{
    auto file_conn = prot::VfsConnection::connect();



    if (file_conn.is_error())
    {
        return -1;
    }

    auto file_res = file_conn.unwrap().open_path((core::Str(path)));

    if (file_res.is_error())
    {
        return -1;
    }

    auto info_res = file_res.unwrap().get_info();

    if (info_res.is_error())
    {
        return -1;
    }

    auto info = core::move(info_res.unwrap());

    buf->st_dev = 0;
    buf->st_ino = 0;
    buf->st_mode = info.is_directory;
    buf->st_nlink = 1;
    buf->st_uid = 0;
    buf->st_gid = 0;
    buf->st_rdev = 0;
    buf->st_size = info.size;
    buf->st_atime = info.accessed_at;
    buf->st_mtime = info.modified_at;
    buf->st_ctime = info.created_at;
    buf->st_blksize = 4096;
    buf->st_blocks = (info.size + 511) / 512;

    file_res.unwrap().close();
    file_conn.unwrap().raw_client().disconnect();
    return -1;
}