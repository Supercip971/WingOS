#include "sys/stat.h"
#include <protocols/vfs/file.hpp>
#include <protocols/vfs/vfs.hpp>

int stat(const char *__restrict path, struct stat *__restrict buf)
{
    auto file_conn_res = prot::VfsConnection::connect();

    if (file_conn_res.is_error())
    {
        return -1;
    }

    auto file_conn = file_conn_res.take();

    auto file_res = file_conn.open_path((core::Str(path)));

    if (file_res.is_error())
    {
        file_conn.raw_client().disconnect();
        return -1;
    }

    auto file = file_res.take();

    auto info_res = file.get_info();

    if (info_res.is_error())
    {
        file.close();
        file_conn.raw_client().disconnect();
        return -1;
    }

    auto info = info_res.take();

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

    file.close();
    file_conn.raw_client().disconnect();
    return 0;
}