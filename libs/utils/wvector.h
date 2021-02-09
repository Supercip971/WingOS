#ifndef WVECTOR_H
#define WVECTOR_H
#include <stdio.h>
#include <stdlib.h>
#include <utils/container.h>
namespace wos
{
    template <typename vtype>
    class vector : container<vtype>
    {
        vtype *buffer = nullptr; // to do : do a direct buffer type
        size_t sz;
        size_t allocated_size;

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
        vector()
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
            const size_t last = sz;
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

        vtype &operator[](size_t idx)
        {
            if (idx > sz)
            {
                printf("out of bound error\n");
                return buffer[0];
            }
            return buffer[idx];
        }
        const vtype operator[](size_t idx) const
        {
            if (idx > sz)
            {
                printf("out of bound error\n");
                return buffer[0];
            }
            return buffer[idx];
        }

        vtype &get(size_t idx) override
        {
            if (idx > sz)
            {
                printf("out of bound error\n");
                return buffer[0];
            }
            return buffer[idx];
        };
        const vtype get(size_t idx) const override
        {
            if (idx > sz)
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

        ~vector()
        {
            clear();
        }

        size_t size() const override
        {
            return sz;
        }
    };
} // namespace wos

#endif // WVECTOR_H
