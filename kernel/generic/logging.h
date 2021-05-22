#ifndef LOGGING_H
#define LOGGING_H
#include <device/debug/com.h>
#include <device/general_device.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <utility.h>
#include <utils/lock.h>
enum log_state
{
    LOG_INFO = 0,
    LOG_DEBUG = 1,
    LOG_ERROR = 2,
    LOG_FATAL = 3,
    LOG_WARNING = 4,
    LOG_NULL = 5,
};

class range_str
{

public:
    const char *rdata;
    size_t rlength;
    range_str(const char *data, size_t size) : rdata(data), rlength(size){};
};
template <class T>
void log_print_element(T element);

template <>
void log_print_element<>(const char *element);

template <>
void log_print_element<>(char *element);

template <>
void log_print_element<>(char element);

template <>
void log_print_element<>(range_str element);

template <>
void log_print_element<>(int64_t element);

template <>
void log_print_element<>(uint64_t element);

template <>
void log_print_element<>(uint32_t element);

template <>
void log_print_element<>(int element);

template <>
void log_print_element<>(uint16_t element);

template <>
void log_print_element<>(uint8_t element);

template <>
void log_print_element<>(void *element);
template <>
void log_print_element<>(long element);

void print_log_str(const char *d, log_state state);

void slog(const char *msg);
template <typename arg1>
constexpr void slog(const char *msg, arg1 &arg)
{
    debug_device *dev = find_device<debug_device>();
    size_t length = 0;
    for (size_t i = 0; i < strlen(msg); i++)
    {
        if (i != 0 && msg[i] == '}' && msg[i - 1] == '{')
        {
            length = i + 1;
            break;
        }
    }
    if (length == 0)
    {
        length = strlen(msg);
        dev->echo_out(msg, length);
        return;
    }
    else
    {
        dev->echo_out(msg, length - 2);
        log_print_element<arg1>(arg);
        dev->echo_out(msg + length, strlen(msg) - length);
    }
}
template <typename arg1, typename... argument>

constexpr void slog(const char *msg, arg1 &arg, argument &...args)
{
    debug_device *dev = find_device<debug_device>();
    size_t length = 0;
    for (size_t i = 0; i < strlen(msg); i++)
    {
        if (i != 0 && msg[i] == '}' && msg[i - 1] == '{')
        {
            length = i + 1;
            break;
        }
    }
    if (length == 0)
    {
        length = strlen(msg);
        dev->echo_out(msg, length);
        return;
    }
    else
    {
        if (length - 2 > 0)
        {
            dev->echo_out(msg, length - 2);
        }
        log_print_element<arg1>(arg);
        return slog(msg + length, args...);
    }
}
void start_log();
void end_log();
template <typename... argument>
constexpr void log(const char *d, log_state state, const char *msg, argument... arg)
{
    start_log();
    print_log_str(d, state);
    slog(msg, arg...);

    end_log();
};

#endif
