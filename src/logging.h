#pragma once
#include <stdint.h>
enum log_state
{
    LOG_INFO = 0,
    LOG_DEBUG = 1,
    LOG_ERROR = 2,
    LOG_FATAL = 3,
    LOG_WARNING = 4
};

class logging
{
public:
    logging();
    void set_log_type(const char *data, log_state log_state);
    logging operator<<(const char *string);
    logging operator<<(uint64_t address);
};

logging log(const char *data, log_state color_mode);
