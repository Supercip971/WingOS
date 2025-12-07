#pragma once 



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

struct FILE 
{
    FILEKind kind; 
    size_t cursor;
    int eof_flag;
    int error_flag;
    int ungetc_buf;  // -1 if empty, otherwise the ungotten char

    union {
        prot::FsFile* file;
        prot::SenderPipe* output; 
        prot::ReceiverPipe* input;
        core::Writer* writer;
        core::Reader* reader;
    };

    FILE() : kind(FILE_KIND_VOID), cursor(0), eof_flag(0), error_flag(0), ungetc_buf(-1) {};
};


void set_stdout_pipe(prot::SenderPipe* pipe);
void set_stderr_pipe(prot::SenderPipe* pipe);
void set_stdin_pipe(prot::ReceiverPipe* pipe);
