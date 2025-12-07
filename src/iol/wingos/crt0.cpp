

#include <libcore/fmt/log.hpp>
#include <string.h>

#include "libc/stdio_fs.hpp"
#include "libcore/str_writer.hpp"

#include "iol/wingos/ipc.hpp"
#include "iol/wingos/syscalls.h"
#include "libcore/fmt/flags.hpp"
#include "mcx/mcx.hpp"
#include "protocols/pipe/pipe.hpp"
#include "protocols/vfs/file.hpp"
#include "protocols/vfs/vfs.hpp"
#include "wingos-headers/startup.hpp"

FILE *stdin;
FILE *stdout;
FILE *stderr;
void set_stdout_pipe(prot::SenderPipe *pipe)
{
    stdout = new FILE();
    stdout->kind = FILE_KIND_OUT;
    stdout->output = pipe;
}

void set_stderr_pipe(prot::SenderPipe *pipe)
{
    stderr = new FILE();
    stderr->kind = FILE_KIND_OUT;
    stderr->output = pipe;
}
void set_stdin_pipe(prot::ReceiverPipe *pipe)
{
    stdin = new FILE();
    stdin->kind = FILE_KIND_IN;
    stdin->input = pipe;
}

extern int main(int argc, char **argv);

__attribute__((weak)) int _main(StartupInfo *context)
{
    return main(context->argc, context->argv);
}

static char _buffer[100];
static size_t _index;

static bool use_stdout = false;

static prot::SenderPipe _stdout_pipe;
static prot::SenderPipe _stderr_pipe;
static prot::ReceiverPipe _stdin_pipe;

static prot::FsFile _pwd;

class WingosLogger : public core::Writer
{
public:
    virtual core::Result<void> write(const char *data, size_t size) override
    {

        for (size_t i = 0; i < size; i++)
        {

            if (data[i] == '\n' || _index >= sizeof(_buffer) - 3)
            {
                _buffer[_index++] = data[i]; // Null-terminate the string
                _buffer[_index++] = '\0';

                if (use_stdout)
                {
                    _stdout_pipe.send(_buffer, _index);
                }
                else
                {
                    sys$debug_log(_buffer);
                }
                _index = 0;
            }
            else
            {
                _buffer[_index++] = data[i];
            }
        }
        return {};
    }
};


class EmptyReader : public core::Reader
{
public:
    virtual core::Result<size_t> read(char* buffer, size_t size) const override
    {
        (void)buffer;
        (void)size;
        return 0ul;
    }
};

asm(
    ".global _start \n"
    "_start: \n"
    "   andq $-16, %rsp \n"
    "   subq $512, %rsp \n"
    "   call _entry_point \n"
    "   \n");

// taken from https://github.com/managarm/managarm/
using InitializerPtr = void (*)();

extern "C" InitializerPtr __init_array_start[];
extern "C" InitializerPtr __init_array_end[];

extern "C" void run_constructors()
{
    auto begin = reinterpret_cast<uintptr_t>(__init_array_start);
    auto end = reinterpret_cast<uintptr_t>(__init_array_end);
    auto count = (end - begin) / sizeof(InitializerPtr);

    for (size_t i = 0; i < count; ++i)
        __init_array_start[i]();
}

static core::WStr *cwd;
char *iol_get_cwd()
{
    return (char *)cwd->view().data();
}

int iol_change_cwd(const char *path)
{

    if (path == NULL)
    {
        return -1;
    }

    if (path[0] == '/')
    {
        _pwd.close();
        _pwd = prot::VfsConnection::connect().unwrap().open_root().unwrap();
        *cwd = "/";
        path++;
    }

    cwd->append(core::Str(path));
    core::Str p = path;
    auto r = p.split('/');
    for (auto &part : r)
    {
        auto &old = _pwd;
        auto n = old.open_file(part);

        if (n.is_error())
        {
            return -1;
        }
        auto new_f = n.unwrap();

        core::swap(new_f, _pwd);

        new_f.close();
    }

    // relative path

    return 0;
}

static WingosLogger logger;
extern "C" __attribute__((weak)) void _entry_point(StartupInfo *context)
{

    asm volatile("andq $-16, %rsp");
    _index = 0;
    logger= {};

    log::provide_log_target(&logger);

    run_constructors();
    cwd = new core::WStr();

    if (context->stdout_handle != 0)
    {
        _stdout_pipe = (prot::SenderPipe::from(
                            Wingos::IpcClient::from(
                                Wingos::Space::self().handle,
                                context->stdout_handle)))
                           .unwrap();
        use_stdout = true;
        set_stdout_pipe(&_stdout_pipe);
    }
    else 
    {
        stdout = new FILE();
        stdout->kind = FILE_KIND_WRITER;
        stdout->writer = &logger;
    }
    if (context->stderr_handle != 0)
    {
        _stderr_pipe = (prot::SenderPipe::from(
                            Wingos::IpcClient::from(
                                Wingos::Space::self().handle,
                                context->stderr_handle)))
                           .unwrap();
        set_stderr_pipe(&_stderr_pipe);
    }
    else  
    {
        stderr = new FILE();
        stderr->kind = FILE_KIND_WRITER;
        stderr->writer = &logger;
        
    }

    if (context->stdin_handle != 0)
    {

        _stdin_pipe = (prot::ReceiverPipe::from(
                           Wingos::IpcClient::from(
                               Wingos::Space::self().handle,
                               context->stdin_handle)))
                          .unwrap();
        set_stdin_pipe(&_stdin_pipe);
    }
    else 
    {
        stdin = new FILE();
        stdin->kind = FILE_KIND_READER;
        stdin->reader = new EmptyReader();
    }

    // Initialize the kernel
    if (_main(context) != 0)
    {
        return;
    }
}