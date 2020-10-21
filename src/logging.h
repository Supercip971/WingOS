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
private:
    static constexpr const char *log_type_table[5] = {
        "\n\033[36m[C%x/T%x] [%s] :\033[0m ",
        "\n\n\033[1m\033[32m[C%x/T%x] [%s] :\033[0m ",
        "\n\033[31;1m[C%x/T%x] [%s] :\033[0m ",
        "\n\033[4m\033[1m\033[31m[C%x/T%x][%s][ FATAL ]: \033[0m",
        "\n\033[31m[C%x/T%x] [%s] :",
    };

public:
    logging();
    void set_log_type(const char *data, log_state log_state);
    logging operator<<(const char *string);
    logging operator<<(uint64_t address);
    logging operator<<(int64_t address);
    logging operator<<(uint32_t number);
    logging operator<<(int number);
    logging operator<<(uint16_t number);
    logging operator<<(char chr);
};

logging log(const char *data, log_state color_mode);
