#pragma once 


#include <stdint.h>
#include <stddef.h>
#include <libcore/type-utils.hpp>
#include <libcore/result.hpp>

namespace core 
{

    enum class SeekFrom 
    {
        SEEK_BEGIN,
        SEEK_CURRENT,
        SEEK_END
    };
    class Seeker 
    {
        public: 

        constexpr virtual Result<void> seek(size_t offset, SeekFrom from = SeekFrom::SEEK_BEGIN) = 0;
        constexpr virtual Result<size_t> tell() = 0;

        constexpr virtual Result<size_t> size()
        {
            size_t current = try$(tell());
            seek(0, SeekFrom::SEEK_END);
            size_t size = tell();
            seek(current, SeekFrom::SEEK_BEGIN);
            return size;
        }

        constexpr virtual Result<size_t> rewind()
        {
            seek(0, SeekFrom::SEEK_BEGIN);
            return tell();
        }

    };

    template<typename T>
    concept Seekable = requires(T* x) 
    {
        {x->seek(0, SeekFrom::SEEK_BEGIN)} -> IsConvertibleToResult<void>;
        {x->tell()} -> IsConvertibleToResult<size_t>;
        {x->size()} -> IsConvertibleToResult<size_t>;
        {x->rewind()} -> IsConvertibleToResult<size_t>;
    };

    static_assert(Seekable<Seeker>);


  
}