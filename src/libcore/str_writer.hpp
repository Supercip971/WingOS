#pragma once 


#include <libcore/io/writer.hpp>
#include <libcore/mem/mem.hpp>
#include <libcore/type-utils.hpp>
#include <libcore/str.hpp>
#include <stddef.h>

#include "libcore/ds/vec.hpp"

namespace core 
{
    class WStr : public MemAccess<char>
    {

        size_t _capacity = 0;
        static constexpr size_t compute_len(const char *str)
        {
            size_t i = 0;
            while (str[i] != '\0')
            {
                i++;
            }
            return i;
        }


        public:

        WStr() : MemAccess(nullptr, 0), _capacity(0) {}

        WStr(WStr const&) = delete;
        WStr &operator=(WStr const&) = delete;
        
       // constexpr WStr(char *str) : MemAccess(str, compute_len(str)), _capacity(compute_len(str)+1) {};

        const Str view() const {
            return Str(this->_data, this->_len);
        }




        static WStr own(char* buffer, size_t len, size_t capacity = 0)
        {
            WStr str ;
            str._data = buffer;
            str._len = len;
            str._capacity = (capacity == 0) ? len + 1 : capacity;
            return str;
        }

        static WStr copy(core::Str const& from )
        {
            char* buffer = (char*)malloc(sizeof(char) * (from.len() + 1));
            for(size_t i = 0; i < from.len(); i++)
            {
                buffer[i] = from[i];
            }
            buffer[from.len()] = '\0';

            
            return WStr::own(buffer, from.len());
        }


        WStr  copy() 
        {
            return WStr::copy(view());
        }

        
        

        WStr(WStr &&other)
        {
            this->_data = other._data;
            this->_len = other._len;
            this->_capacity = other._capacity;
            other._data = nullptr;
            other._len = 0;
            other._capacity = 0;
        }

        WStr &operator=(WStr &&other)
        {
            if(this == &other) {
                return *this;
            }
            if(this->_data != nullptr)
            {
                free(this->_data);
            }
            this->_data = other._data;
            this->_len = other._len;
            this->_capacity = other._capacity;
            other._data = nullptr;
            other._len = 0;
            return *this;
        }
        WStr &operator=(const char *str)
        {
            if(this->_capacity > compute_len(str)+1)
            {
                this->_len = compute_len(str);
                for(size_t i = 0; i < this->_len; i++)
                {
                    this->_data[i] = str[i];
                }
                this->_data[this->_len] = '\0';
                return *this;
            }

            *this = WStr::copy(core::Str(str));
            return *this;
        }

        bool put(char v)
        {

            if(this->_capacity < this->len() + 2)
            {
                this->_capacity = (this->len() + 2) * 2;
                this->_data = (char*)realloc(this->_data, this->_capacity);
                
            }
            if(this->_data == nullptr)
            {
                this->_len = 0;
                return false;
            }
            this->_data[this->_len] = v;
            this->_data[this->_len + 1] = '\0';
            this->_len += 1;
            return true;
        }
        bool append(core::Str const& str)
        {
            if(str.len() == 0)
            {
                return true;
            }
            if(this->_capacity < this->len() + str.len() + 1)
            {
                this->_capacity = (this->len() + str.len() + 1) * 2;
                this->_data = (char*)realloc(this->_data, this->_capacity);
            }
            
            if(this->_data == nullptr)
            {
                this->_len = 0;
                return false;
            }
            for(size_t i = 0; i < str.len(); i++)
            {
                this->_data[this->_len + i] = str[i];
            }
            this->_data[this->_len + str.len()] = '\0';
            this->_len += str.len();
            return true;
        }


        constexpr Result<void> write(const char* data, size_t size)
        {
            this->append(core::Str(data, size));
            return {};
        }

        constexpr Result<void> write(core::Str str)
        {
            return write(str.data(), str.len());
        }

        void clear() 
        {
            
            _len = 0;
        }

        void release()
        {
            if(_data != nullptr)
            {
                free(_data);
                _data = nullptr;
                _len = 0;
                _capacity = 0;
            }
        }
       ~WStr() 
        {
            release();
        }
    };


    static_assert(Writable<WStr>);
}



