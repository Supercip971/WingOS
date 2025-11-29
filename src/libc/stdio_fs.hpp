#pragma once 



#include "protocols/pipe/pipe.hpp"
#include "protocols/vfs/file.hpp"

enum FILEKind 
{
    FILE_KIND_VOID = 0,
    FILE_KIND_FILE,
    FILE_KIND_OUT,
    FILE_KIND_IN
};

struct FILE 
{
    FILEKind kind; 
    size_t cursor;

    union {
        prot::FsFile file;
        prot::SenderPipe* output; 
        prot::ReceiverPipe* input;
    };

    FILE() : kind(FILE_KIND_VOID), cursor(0) {};
};


void set_stdout_pipe(prot::SenderPipe* pipe);
void set_stderr_pipe(prot::SenderPipe* pipe);
void set_stdin_pipe(prot::ReceiverPipe* pipe);
