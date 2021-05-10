#include "file.h"
#include <kern/syscall.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <utils/string_util.h>
namespace sys
{

    file stdin;
    file stdout;
    file stderr;
    file::file()
    {
        opened = false;
    }
    file::file(const utils::string path)
    {
        opened = false;
        open(path);
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
    size_t file::write(const uint8_t *buffer, uint64_t length)
    {
        if (opened == false)
        {
            printf("file is not opened \n");
            return 0;
        }

        size_t writed = sys$write(fid, buffer, length);
        fcurrent_seek_pos += length;
        return writed;
    }

    void file::open(const utils::string path)
    {
        if (opened == true)
        {
            close();
        }
        fcurrent_seek_pos = 0;

        opened = true;
        fpath = path;
        fid = sys$open(fpath.c_str(), 0, 0);
        if (fid == -1)
        {
            opened = false;
            close();
        }
    }
    void file::close()
    {
        if (opened)
        {
            sys$close(fid);

            opened = false;
        }
        fpath = "";
        fid = 0;
    }
    uint64_t file::get_file_length()
    {
        if (opened == false)
        {
            return 0;
        }
        fcurrent_seek_pos = sys$lseek(fid, 0, SEEK_CUR);
        size_t size = sys$lseek(fid, 0, SEEK_END);
        sys$lseek(fid, fcurrent_seek_pos, SEEK_SET);

        return size;
    }

    file get_process_stdf(int idx, pid_t pid)
    {
        char *temp = (char *)malloc(255);
        char *path = (char *)malloc(255);
        int str_idx = 0;
        memcpy(path + str_idx, "/proc/", strlen("/proc/"));
        str_idx += strlen("/proc/");

        utils::int_to_string<int>(temp, 'd', pid);
        memcpy(path + str_idx, temp, strlen(temp));
        str_idx += strlen(temp);

        memcpy(path + str_idx, "/fd/", strlen("/fd/"));
        str_idx += strlen("/fd/");

        utils::int_to_string<int>(temp, 'd', idx);
        memcpy(path + str_idx, temp, strlen(temp));
        str_idx += strlen(temp);

        file f = file(path);
        free(path);
        free(temp);
        return f;
    }
} // namespace sys
