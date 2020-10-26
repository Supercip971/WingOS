#include "file.h"
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
        fcurrent_seek_pos = at;
    }
    uint8_t *file::read(uint8_t *buffer, uint64_t length)
    {
        if (opened == false)
        {
            printf("file is not opened \n");
            return nullptr;
        }
        read_file(fid, buffer, fcurrent_seek_pos, length);
        return buffer;
    }

    void file::open(const char *path)
    {
        if (opened == true)
        {
            close();
        }
        opened = true;
        fpath = path;
        fid = file_open(path, "");
        file_info = get_file_information(fid);
    }
    void file::close()
    {
        opened = false;
        file_close(fid);
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
