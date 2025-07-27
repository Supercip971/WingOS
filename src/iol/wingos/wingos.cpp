

#include <libcore/fmt/log.hpp>
#include <string.h>
#include "mcx/mcx.hpp"

#include "iol/wingos/syscalls.h"

extern int _main(mcx::MachineContext* context);

static char buffer[256];
static size_t _index = 0;
class WingosLogger : public core::Writer
{
public:
    virtual core::Result<void> write(const char *data, size_t size) override
    {
        
        for(size_t i = 0; i < size; i++)
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

extern "C" void _start(mcx::MachineContext* context)
{
    _index = 0;

    WingosLogger logger;
    log::provide_log_target(&logger);

    // Initialize the kernel
    if (_main(context) != 0)
    {
        return;
    }


}