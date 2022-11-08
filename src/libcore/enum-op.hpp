#pragma once
#include <libcore/type-utils.hpp>
#include <stdint.h>

#define ENUM_OP$(ENUM)                                                                                         \
    inline constexpr ENUM                                                                                      \
    operator&(ENUM x, ENUM y)                                                                                  \
    {                                                                                                          \
        return static_cast<ENUM>(static_cast<core::UnderlyingType<ENUM>>(x) & static_cast<core::UnderlyingType<ENUM>>(y)); \
    }                                                                                                          \
                                                                                                               \
    inline constexpr ENUM                                                                                      \
    operator|(ENUM x, ENUM y)                                                                                  \
    {                                                                                                          \
        return static_cast<ENUM>(static_cast<core::UnderlyingType<ENUM>>(x) | static_cast<core::UnderlyingType<ENUM>>(y)); \
    }                                                                                                          \
                                                                                                               \
    inline constexpr ENUM                                                                                      \
    operator^(ENUM x, ENUM y)                                                                                  \
    {                                                                                                          \
        return static_cast<ENUM>(static_cast<core::UnderlyingType<ENUM>>(x) ^ static_cast<core::UnderlyingType<ENUM>>(y)); \
    }                                                                                                          \
                                                                                                               \
    inline constexpr ENUM                                                                                      \
    operator~(ENUM x)                                                                                          \
    {                                                                                                          \
        return static_cast<ENUM>(~static_cast<core::UnderlyingType<ENUM>>(x));                                       \
    }                                                                                                          \
                                                                                                               \
    inline ENUM &                                                                                              \
    operator&=(ENUM &x, ENUM y)                                                                                \
    {                                                                                                          \
        x = x & y;                                                                                             \
        return x;                                                                                              \
    }                                                                                                          \
                                                                                                               \
    inline ENUM &                                                                                              \
    operator|=(ENUM &x, ENUM y)                                                                                \
    {                                                                                                          \
        x = x | y;                                                                                             \
        return x;                                                                                              \
    }                                                                                                          \
                                                                                                               \
    inline ENUM &                                                                                              \
    operator^=(ENUM &x, ENUM y)                                                                                \
    {                                                                                                          \
        x = x ^ y;                                                                                             \
        return x;                                                                                              \
    }



namespace core 
{
    template <typename T>
    core::UnderlyingType<T>
    underlying_value(T&& x)
    {
        return static_cast<core::UnderlyingType<T>>(x);
    }
}