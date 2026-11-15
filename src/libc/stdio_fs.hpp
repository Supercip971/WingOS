#pragma once

#include "libcore/str_writer.hpp"

#include "libcore/io/reader.hpp"
#include "libcore/io/writer.hpp"
#include "protocols/pipe/pipe.hpp"
#include "protocols/vfs/file.hpp"

enum FILEKind
{
    FILE_KIND_VOID = 0,
    FILE_KIND_FILE,
    FILE_KIND_OUT,
    FILE_KIND_IN,
    FILE_KIND_WRITER,
    FILE_KIND_READER
};

struct _IO_FILE
{
    FILEKind kind{FILE_KIND_VOID};
    size_t cursor{0};
    size_t end;
    int eof_flag{0};
    int error_flag{0};
    int ungetc_buf{-1}; // -1 if empty, otherwise the ungotten char
    fc::WStr buffer;

    union
    {
        prot::FsFile *file;

        prot::SenderPipe *output;
        prot::ReceiverPipe *input;
        fc::Writer *writer;
        fc::Reader *reader;
    };

    _IO_FILE() {};
};

void set_stdout_pipe(prot::SenderPipe *pipe);
void set_stderr_pipe(prot::SenderPipe *pipe);
void set_stdin_pipe(prot::ReceiverPipe *pipe);
