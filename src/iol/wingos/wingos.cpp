

#include <libcore/fmt/log.hpp>
#include <string.h>

#include "iol/wingos/ipc.hpp"
#include "iol/wingos/syscalls.h"
#include "mcx/mcx.hpp"
#include "protocols/pipe/pipe.hpp"
#include "wingos-headers/startup.hpp"

extern int main(int argc, char **argv);

__attribute__((weak)) int _main(StartupInfo *context)
{
    return main(context->argc, context->argv);
}

static char buffer[100];
static size_t _index;

static bool use_stdout = false;

static prot::SenderPipe _stdout_pipe;
static prot::SenderPipe _stderr_pipe;
static prot::ReceiverPipe _stdin_pipe;

class WingosLogger : public core::Writer
{
public:
    virtual core::Result<void> write(const char *data, size_t size) override
    {

        for (size_t i = 0; i < size; i++)
        {

            if (data[i] == '\n' || _index >= sizeof(buffer) - 3)
            {
                buffer[_index++] = data[i]; // Null-terminate the string
                buffer[_index++] = '\0';

                if (use_stdout)
                {
                    _stdout_pipe.send(buffer, _index);
                }
                else
                {
                    sys$debug_log(buffer);
                }
                _index = 0;
            }
            else
            {
                buffer[_index++] = data[i];
            }
        }
        return {};
    }
};

asm(
    ".global _start \n"
    "_start: \n"
    "   andq $-16, %rsp \n"
    "   subq $512, %rsp \n"
    "   call _entry_point \n"
    "   \n");

extern "C" __attribute__((weak))  void _entry_point(StartupInfo *context)
{

    asm volatile("andq $-16, %rsp");
    _index = 0;

    WingosLogger logger;
    log::provide_log_target(&logger);


    if (context->stdout_handle != 0)
    {
        _stdout_pipe = (prot::SenderPipe::from(
                            Wingos::IpcClient::from(
                                Wingos::Space::self().handle,
                                context->stdout_handle)))
                           .unwrap();
        use_stdout = true;
    }

    if (context->stderr_handle != 0)
    {
        _stderr_pipe = (prot::SenderPipe::from(
                            Wingos::IpcClient::from(
                                Wingos::Space::self().handle,
                                context->stderr_handle)))
                           .unwrap();
    }

    if (context->stdin_handle != 0)
    {

        _stdin_pipe = (prot::ReceiverPipe::from(
                           Wingos::IpcClient::from(
                               Wingos::Space::self().handle,
                               context->stdin_handle)))
                          .unwrap();
    }

    // Initialize the kernel
    if (_main(context) != 0)
    {
        return;
    }
}