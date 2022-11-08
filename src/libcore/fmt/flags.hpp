#pragma once 

#include "libcore/type-utils.hpp"
namespace fmt 
{
template <typename T>
struct FormatFlags 
{
    bool is_hex;
    T&& value;

    template<typename C>
    constexpr FormatFlags<C> forward_flags(C && new_value)
    {
        return FormatFlags<C>{
            is_hex, 
            core::forward<C>(new_value),
            };
    }
};

typedef enum 
{
    FMT_HEX,
} FmtFlag;


template <typename T> 
constexpr FormatFlags<T> operator|(T&& a, FmtFlag flag) 
{
    switch (flag) 
    {
    case FMT_HEX:
        return FormatFlags<T>{
            .is_hex = true, 
            .value = core::forward<T>(a),
            };
    }
    return FormatFlags<T>{ 
        .is_hex = false, 
        .value = core::forward<T>(a),
        };
}
template<typename T>
constexpr FormatFlags<T> operator|(FormatFlags<T>&& a, FmtFlag flag) 
{
    switch (flag) 
    {
    case FMT_HEX:
        a.is_hex = true;
        break;
    }
    return a;
}


}