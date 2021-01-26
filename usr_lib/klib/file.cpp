#include "file.h"
#include <klib/syscall.h>
#include <stdio.h>
namespace sys
{

    file::file()
    {
        opened = false;
    }
    file::file(const char *path)
    {
        opened = false;
        fpath = path;
        open(fpath);
    }

    void file::seek(uint64_t at)
    {
        sys$lseek(fid, at, SEEK_SET);
    }
    size_t file::read(uint8_t *buffer, uint64_t length)
    {
        if (opened == false)
        {
            printf("file is not opened \n");
            return 0;
        }

        size_t readed = sys$read(fid, buffer, length);
        fcurrent_seek_pos += length;
        return readed;
    }

    void file::open(const char *path)
    {
        if (opened == true)
        {
            close();
        }
        opened = true;
        fpath = path;
        fid = sys$open(path, 0, 0);
    }
    void file::close()
    {
        opened = false;
        sys$close(fid);
    }
    uint64_t file::get_file_length()
    {
        if (opened == false)
        {
            return 0;
        }
        return file_info.size; // not implemented
    }
} // namespace sys
