#pragma once
#include <com.h>
#include <print.h>
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
    void set_log_type(const char *data, log_state log_state);
    template <typename T>
    logging operator<<(T data);
};

template <>
logging logging::operator<<(int64_t data);
template <>
logging logging::operator<<(uint64_t data);
template <>
logging logging::operator<<(const char *data);
template <>
logging logging::operator<<(void *data);
template <>
logging logging::operator<<(char *data);
template <>
logging logging::operator<<(char data);
template <>
logging logging::operator<<(unsigned char data);
logging log(const char *data, log_state color_mode);
