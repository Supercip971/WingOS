#ifndef WARRAY_H
#define WARRAY_H
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <utils/container.h>
namespace utils
{
    template <typename vtype, size_t array_count>
    class array : container<vtype>
    {

        vtype buffer[array_count];

        inline bool is_bounded(size_t val) const
        {

            if (val > array_count)
            {
                return false;
            }
            return true;
        }

    public:
        array()
        {
        }
        array(const vtype new_value)
        {
            for (size_t i = 0; i < array_count; i++)
            {
                buffer[i] = new_value;
            }
        }
        array(const vtype &new_value)
        {
            for (size_t i = 0; i < array_count; i++)
            {
                buffer[i] = new_value;
            }
        }
        void fill(const vtype &value)
        {
            for (size_t i = 0; i < array_count; i++)
            {
                buffer[i] = value;
            }
        }
        vtype &operator[](size_t idx)
        {

            if (!is_bounded(idx))
            {
                printf("out of bound error\n");
                return buffer[0];
            }
            return buffer[idx];
        }
        const vtype operator[](size_t idx) const
        {
            if (!is_bounded(idx))
            {
                printf("out of bound error\n");
                return buffer[0];
            }
            return buffer[idx];
        }

        vtype &get(size_t idx) override
        {
            if (!is_bounded(idx))
            {
                printf("out of bound error\n");
                return buffer[0];
            }
            return buffer[idx];
        };

        const vtype get(size_t idx) const override
        {
            if (!is_bounded(idx))
            {
                printf("out of bound error\n");
                return buffer[0];
            }
            return buffer[idx];
        };

        vtype *raw()
        { // really not recommanded
            return buffer;
        }

        constexpr size_t size() const override
        {
            return array_count;
        }

        size_t capacity() const
        {
            return array_count * sizeof(vtype);
        }
    };
} // namespace utils

#endif // WARRAY_H
