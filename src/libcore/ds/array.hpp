#pragma once 

#include <libcore/buffer.hpp>

namespace core 
{
    template <typename T, int size>
    class Array 
    {
    private:
        T _data[size];

    public:
        constexpr Array() : _data{} {}
 
        template<typename A> 
        constexpr Array(A a) : _data{a} {}

        template<typename ...A> 
        constexpr Array(A... a) : _data{a...} {}



        using Type = T;

        constexpr operator bool() const
        {
            return size > 0;
        }

        constexpr T &operator[](int index)
        {
            return _data[index];
        }

        constexpr const T &operator[](int index) const
        {
            return _data[index];
        }

        constexpr T *data()
        {
            return _data;
        }

        constexpr const T *data() const
        {
            return _data;
        }

        constexpr size_t len() const
        {
            return size;
        }

        constexpr T *begin()
        {
            return _data;
        }

        constexpr T *end()
        {
            return _data + size;
        }

        constexpr const T *begin() const
        {
            return _data;
        }

        constexpr const T *end() const
        {
            return _data + size;
        }

        constexpr MemView<const T> view() const
        {
            return MemView<const T>(_data, size);
        }


    };

static_assert(Viewable<Array<char, 10>>);

static_assert(sizeof(Array<char,10>) == sizeof(char[10]));
static_assert(sizeof(Array<float,10>) == sizeof(float[10]));


}