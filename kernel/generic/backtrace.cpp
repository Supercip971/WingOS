#include "backtrace.h"
#include <logging.h>

backtrace::backtrace()
{
}

void backtrace::add_entry(const backtrace_entry_type added_entry)
{
    if (entry[backtrace_max_entry_count - 1] != added_entry)
    {
        for (int i = 0; i < backtrace_max_entry_count - 1; i++)
        {
            entry[i] = entry[i + 1];
        }
        entry[backtrace_max_entry_count - 1] = added_entry;
    }
}

void backtrace::dump_backtrace()
{
    for (int i = backtrace_max_entry_count - 1; i >= 0; i--)
    {

        log("backtrace", LOG_INFO) << "id " << i << " = " << entry[i];
    }
}
