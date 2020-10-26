#pragma once
#include <klib/kernel_file_system.h>
namespace sys
{

    class file
    {
        uint64_t fid;
        uint64_t fcurrent_seek_pos;
        bool opened = false;
        const char *fpath;
        file_information file_info;

    public:
        file();
        file(const char *path);

        void seek(uint64_t at);
        uint8_t *read(uint8_t *buffer, uint64_t length);

        void open(const char *path);
        void close();
        uint64_t get_file_length();
    };
} // namespace sys
