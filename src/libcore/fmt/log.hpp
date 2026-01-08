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

void log_lock();
void log_release();
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
    core::Str sub = str.sub_penultimate_char('/');
    if (sub)
    {
        return sub.substr(1);
    }
    return str;
}

#define __LOG_FILENAME__ ([]() {constexpr const char* _s = __FILE__; return log::file_name(_s); })()


#ifdef KERNEL
inline consteval core::Str color_from_filename(const char *s)
{
    (void)s;
    return core::Str("5");
}
#else
inline consteval core::Str color_from_filename(const char *s)
{

    core::Str str2 = log::file_name(s);
    core::Str str3 = str2.remove_after('/');
    size_t hash = 0;
    for (size_t i = 0; i < str3.len(); i++)
    {
        hash = hash * 31 + (str3[i] - 'a' + 1);
    }
    const char *colors[] = {
        "2", "3", "4",  "6", "9", "10", "11", "12", "13", "14",
    "21", "26", "28", "30", "33", "34", "36", "37", "38", "39",
"40", "41", "42", "43", "44", "45", "46", "47", "48", "49",
"50", "51", "56", "57", "58", "62", "63", "64", "68", "69", "70",
"74", "75", "76", "77", "78", "79", "80",
"81", "82", "83", "84", "85", "86", "87",
"90", "91", "92", "93", "94", "95", "96",
"97", "98", "99", "100", "101", "102", "103", "104", "105", "106",
"107", "108", "109", "110", "111", "112", "113", "114", "115", "116",
"117", "118", "119", "120", "121", "122", "123", "125", "126", "127",
"128", "129", "130", "131", "132", "133", "134", "135", "136", "137", "138",
"139", "140", "141", "142", "143", "144", "145", "146", "147", "148", "149",
"150", "151", "152", "153", "154", "155", "156", "157", "158", "159",
"162", "163", "164", "165", "166", "167", "168", "169", "170",
    "171", "172", "173", "174", "175", "176", "177", "178", "179",
    "180", "181", "182", "183", "184", "185", "186", "187", "188", "189",
    "190", "191", "192", "193", "194", "195", "198", "199",
    "200", "201", "202", "203", "204", "205", "206", "207", "208", "209",
    "210", "211", "212", "213", "214", "215", "216", "217", "218", "219",
    "220", "221", "222", "223", "224", "225", "226", "227", "228", "229"
    };
    return core::Str(colors[hash % (sizeof(colors) / sizeof(colors[0]))]);
}
#endif

#ifndef LOG_DEFAULT_COLOR

#    define LOG_DEFAULT_COLOR log::color_from_filename(__FILE__)


#endif


#ifndef NO_LOG_COLOR
#    define log$(...)                                   \
        log("\033[38;5;{}m[{}]:\033[0m ",  LOG_DEFAULT_COLOR,  __LOG_FILENAME__); \
        log::log(__VA_ARGS__);                          \
        log::log("\n")

#    define err$(...)                                   \
        log("\033[91m[{}] (<!!!!>):\033[0m ", __LOG_FILENAME__); \
        log::log(__VA_ARGS__);                          \
        log::log("\n")

#    define warn$(...)                                  \
        log("\033[93m[{}] (<!>):\033[0m ", __LOG_FILENAME__); \
        log::log(__VA_ARGS__);                          \
        log::log("\n")

#else
#    define log$(...)                    \
        log("[{}]: ", __LOG_FILENAME__); \
        log::log(__VA_ARGS__);           \
        log::log("\n")

#endif

} // namespace log

template <typename ValT, typename ErrT>
inline void core::Result<ValT, ErrT>::assert()
{
    if (is_error())
    {
        log::log("Result assert failed: {}", _error);

        while (true)
        {
        };
    }
}

// Provide the missing specialization for Result<void, ErrT>::assert().
// Without this, some builds end up with an undefined symbol at link time.
template <typename ErrT>
inline void core::Result<void, ErrT>::assert()
{
    if (is_error())
    {
        log::log("Result assert failed: {}", _error);

        while (true)
        {
        };
    }
}