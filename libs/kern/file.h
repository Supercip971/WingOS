#pragma once
#include <stddef.h>
#include <stdint.h>
#include <utils/wstring.h>
namespace sys
{

    class file
    {
        uint64_t fid;
        uint64_t fcurrent_seek_pos;
        bool opened = false;
        utils::string fpath;

    public:
        file();
        file(const utils::string path);

        void seek(uint64_t at);

        uint64_t get_cursor_pos() { return fcurrent_seek_pos; };
        size_t read(uint8_t *buffer, uint64_t length);
        size_t write(const uint8_t *buffer, uint64_t length);
        bool is_openned() const { return opened; };
        void open(const utils::string path);
        void close();
        uint64_t get_file_length();
    };
    extern file stdin;
    extern file stdout;
    extern file stderr;

    file get_process_stdf(int idx, int pid);
} // namespace sys
