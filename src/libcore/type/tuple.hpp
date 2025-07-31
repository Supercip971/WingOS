#pragma once 

#include <stddef.h>
#include "libcore/type/trait.hpp"
namespace core 
{


template<typename... Args>
struct TypeTuple 
{
    static constexpr size_t size = sizeof...(Args);

    
};


template<typename Arg0, typename... Args>
struct TypeTuple<Arg0, Args...>
{
    using Type0 = Arg0;
    using TypeRest = TypeTuple<Args...>;

    static constexpr size_t count = 1 + TypeRest::count;

    template<size_t Index> 
    using TypeAt = core::ConditionalType<Index == 0, Arg0, typename TypeRest::template TypeAt<Index - 1>>;

};
template<typename Arg0>
struct TypeTuple<Arg0>
{
    static constexpr size_t count = 1;
    using Type0 = Arg0;

    template<size_t Index>
    using TypeAt = Arg0;
    
};

};