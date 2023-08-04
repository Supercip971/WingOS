#pragma once
#include "libcore/type-utils.hpp"
#define HAS_LOGGING
#include <libcore/fmt/fmt.hpp>
#include <libcore/io/writer.hpp>
#include <libcore/result.hpp>
#include <libcore/str.hpp>
namespace log
{

void provide_log_target(core::Writer *writer);

core::Writer *log_target();

template <typename S>
constexpr void log_impl(S &&arg)
{
    log_target()->write(core::Str(arg));
}

template <typename S, typename... Args>
constexpr void log_impl(S &&arg, Args &&...args)
{
    log_target()->write(core::Str(arg));
    log_impl((args)...);
}

template <typename Fmt, typename... Args>
constexpr void log(Fmt &&fmt, Args &&...args)
{

    fmt::format(*log_target(), (fmt), core::forward<Args>(args)...);
}

inline constexpr core::Str file_name(const char *s)
{
    core::Str str(s);
    core::Str sub = str.sub_last_char('/');
    if (sub)
    {
        return sub.substr(1);
    }
    return str;
}

#define __LOG_FILENAME__ ([]() {constexpr const char* _s = __FILE__; return log::file_name(_s); })()

#ifndef NO_LOG_COLOR
#    define log$(...)                                                \
        log("\033[95m[{}:{}]:\033[0m ", __LOG_FILENAME__, __LINE__); \
        log::log(__VA_ARGS__);                                       \
        log::log("\n")

#    define err$(...)                                                \
        log("\033[91m[{}:{}]:\033[0m ", __LOG_FILENAME__, __LINE__); \
        log::log(__VA_ARGS__);                                       \
        log::log("\n")

#    define warn$(...)                                                \
        log("\033[93m[{}:{}]:\033[0m ", __LOG_FILENAME__, __LINE__); \
        log::log(__VA_ARGS__);                                       \
        log::log("\n")




#else
#    define log$(...)                                 \
        log("[{}:{}]: ", __LOG_FILENAME__, __LINE__); \
        log::log(__VA_ARGS__);                        \
        log::log("\n")

#endif

} // namespace log

template <typename ValT, typename ErrT>
inline void core::Result<ValT, ErrT>::assert()
{
    if (_error.has_value())
    {

        log::log("Result assert failed: ", _error.unwrap());

        while (true)
        {
        };
    }
}
template <typename ErrT>
inline void core::Result<void, ErrT>::assert()
{
    if (_error.has_value())
    {

        log::log("Result assert failed: ", _error.unwrap());

        while (true)
        {
        };
    }
}
