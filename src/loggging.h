#pragma once
#include <stdint.h>
enum log_state
{
    LOG_INFO = 0,
    LOG_DEBUG = 1,
    LOG_ERROR = 2
};

class loggging
{
public:
    loggging();
    void set_log_type(const char *data, log_state log_state);
    loggging operator<<(const char *string);
    loggging operator<<(uint64_t address);
};

loggging log(const char *data, log_state color_mode);
