

#include <libcore/fmt/log.hpp>
#include <string.h>

#include "iol/wingos/syscalls.h"
#include "mcx/mcx.hpp"
#include "wingos-headers/startup.hpp"

extern int main(int argc, char** argv);

__attribute__((weak))
int _main(StartupInfo *context)
{
    return main(context->argc, context->argv);
}

static char buffer[512];
static size_t _index;
class WingosLogger : public core::Writer
{
public:
    virtual core::Result<void> write(const char *data, size_t size) override
    {

        for (size_t i = 0; i < size; i++)
        {
            if (data[i] == '\n' || _index >= sizeof(buffer) - 3)
            {
                buffer[_index++] = '\n'; // Null-terminate the string
                buffer[_index++] = '\0';
                sys$debug_log(buffer);
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




    __attribute__((weak))
extern "C" void _entry_point(StartupInfo *context)
{

    asm volatile("andq $-16, %%rsp" ::: "rsp");
    _index = 0;

    WingosLogger logger;
    log::provide_log_target(&logger);

    // Initialize the kernel
    if (_main(context) != 0)
    {
        return;
    }
}