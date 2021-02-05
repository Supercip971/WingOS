#ifndef WVECTOR_H
#define WVECTOR_H

#include <stdlib.h>
namespace fth
{
    template <typename vtype>
    class wvector
    {
        vtype *buffer = nullptr; // to do : do a direct buffer type
        unsigned int sz;
        unsigned int allocated_size;

        void create()
        {
            sz = 0;
            allocated_size = sizeof(vtype) * 4;
            buffer = (vtype *)malloc(allocated_size);
        }
        void increase()
        {
            if (buffer == nullptr)
            {
                create();
            }
            sz++;
            if (sz >= (allocated_size / sizeof(vtype)))
            {
                allocated_size += sizeof(vtype) * 4;
                buffer = (vtype *)realloc((void *)buffer, allocated_size);
            }
        }

    public:
        wvector()
        {
            sz = 0;
            allocated_size = 0;
            buffer = nullptr;
        }

        operator bool()
        {
            if (buffer == nullptr || sz == 0 || allocated_size == 0)
            {
                return false;
            }
            return true;
        }

        void push_back(vtype data)
        {
            const unsigned int last = sz;
            increase();
            buffer[last] = data;
        }

        void clear()
        {
            if (buffer != nullptr)
            {

                free(buffer);
            }
            allocated_size = 0;
            sz = 0;
        }

        vtype &operator[](unsigned int idx)
        {
            if (idx > sz)
            {
                return buffer[0];
            }
            return buffer[idx];
        }

        vtype *buf()
        { // really not recommanded
            return buffer;
        }

        ~wvector()
        {
            clear();
        }

        unsigned int size()
        {
            return sz;
        }
    };
} // namespace fth

#endif // WVECTOR_H
