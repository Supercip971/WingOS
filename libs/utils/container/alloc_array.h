#ifndef ALLOC_ARRAY_H
#define ALLOC_ARRAY_H
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <utils/container/container.h>
#include <utils/type_traits.h>
namespace utils
{

    // an alloc array is an alternative to vector, used/will be used for things like message list
    // were we want a list with the "status" of the element, if it is used or usable
    // we can do msg_list.alloc() to find a free message descriptor and use it, and when  we do not want to use it we can free it with msg_list.free()
    // it may be used instead of a vector for passing element by pointer without reallocating the buffer, it is important with smp, as a cpu can realloc a buffer when the second use it
    // we can use lock but i don't want to lock everything every time we just want to look something

    template <typename vtype, size_t array_count>
    class alloc_array : public container<vtype>
    {

        vtype _buffer[array_count];
        bool _status[array_count]; // maybe use a bitmap ? but if I use a bitmap the array_count must be a multiple of 8
        size_t _element_count;
        size_t _last_free;
        inline bool is_bounded(size_t val) const
        {

            if (val > array_count)
            {
                return false;
            }
            return true;
        }

    public:
        alloc_array()
        {
            for (size_t i = 0; i < array_count; i++)
            {
                _status[i] = false;
            }
            _element_count = 0;
        }

        alloc_array(const vtype new_value)
        {
            for (size_t i = 0; i < array_count; i++)
            {
                _buffer[i] = new_value;
                _status[i] = false;
            }
            _element_count = 0;
        }

        vtype &operator[](size_t idx)
        {

            if (!is_bounded(idx))
            {
                printf("out of bound error\n");
                return _buffer[0];
            }
            if (_status[idx])
            {
                return _buffer[idx];
            }
            printf("free idx error\n");
            return _buffer[0];
        }

        const vtype &operator[](size_t idx) const
        {
            if (!is_bounded(idx))
            {
                printf("out of bound error\n");
                return _buffer[0];
            }
            if (_status[idx])
            {
                return _buffer[idx];
            }
            printf("free idx error\n");
            return _buffer[0];
        }

        vtype &get(size_t idx) override
        {
            if (!is_bounded(idx))
            {
                printf("out of bound error\n");
                return _buffer[0];
            }
            if (_status[idx])
            {
                return _buffer[idx];
            }
            printf("free idx error\n");
            return _buffer[0];
        };

        const vtype &get(size_t idx) const override
        {
            if (!is_bounded(idx))
            {
                printf("out of bound error\n");
                return _buffer[0];
            }
            if (_status[idx])
            {
                return _buffer[idx];
            }
            printf("free idx error\n");
            return _buffer[0];
        };

        bool status(size_t idx)
        {

            if (!is_bounded(idx))
            {
                printf("out of bound error\n");
                return false;
            }
            return _status[idx];
        }

        vtype *raw()
        { // really not recommanded
            return _buffer;
        }

        constexpr size_t size() const override
        {
            return array_count;
        }

        size_t capacity() const
        {
            return array_count * sizeof(vtype);
        }
        size_t allocated_element_count() const { return _element_count; };

        long alloc()
        {

            for (size_t i = 0; i < array_count; i++)
            {
                if (_status[i] == false)
                {
                    _status[i] = true;
                    _element_count++;
                    return (long)i;
                };
            }
            printf("can't allocate a new entry in alloc_array\n");
            return 0;
        }

        bool free(size_t idx)
        {

            if (!is_bounded(idx))
            {
                printf("out of bound error\n");
                return false;
            }
            if (!_status[idx])
            {
                printf("error: trying to free an already free array element\n");
                return false;
            }
            _element_count--;
            _status[idx] = false;
            return true;
        }

        template <typename func>
        bool foreach_entry(func call)
        {
            for (size_t i = 0; i < array_count; i++)
            {
                if (_status[i] == true)
                {
                    call(_buffer[i]);
                };
            }
            return true;
        }
    };

}; // namespace utils

#endif // ALLOC_ARRAY_H
