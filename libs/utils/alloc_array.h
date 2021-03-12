#ifndef ALLOC_ARRAY_H
#define ALLOC_ARRAY_H
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <utils/container.h>
#include <utils/type_traits.h>
namespace utils
{

// an alloc array is an alternative to vector, used/will be used for things like message list
// were we want a list with the "status" of the element, if it is used or usable
// we can do msg_list.alloc() to find a free message descriptor and use it, and when  we do not want to use it we can free it with msg_list.free()
// it may be used instead of a vector for passing element by pointer without reallocating the buffer, it is important with smp, as a cpu can realloc a buffer when the second use it
// we can use lock but i don't want to lock everything every time we just want to look something


    template <typename  T>
    struct alloc_array_member
    {
        T raw;
        bool status;
    };

    template <typename vtype, size_t array_count>
    class alloc_array : public container<vtype>
    {

        alloc_array_member<vtype> buffer[array_count];
        size_t element_count;
        size_t last_free;
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
            for(size_t i = 0; i < array_count; i++){
                buffer[i].status = false;
            }
            element_count = 0;
        }

        alloc_array(const vtype new_value)
        {
            for (size_t i = 0; i < array_count; i++)
            {
                buffer[i].raw = new_value_tmp;
                buffer[i].status = false;
            }
            element_count = 0;
        }

        vtype &operator[](size_t idx)
        {

            if (!is_bounded(idx))
            {
                printf("out of bound error\n");
                return buffer[0].raw;
            }
            if(buffer[idx].status){
                return buffer[idx].raw;

            }
            printf("free idx error\n");
            return buffer[0].raw;
        }

        const vtype &operator[](size_t idx) const
        {
            if (!is_bounded(idx))
            {
                printf("out of bound error\n");
                return buffer[0].raw;
            }
            if(buffer[idx].status){
                return buffer[idx].raw;

            }
            printf("free idx error\n");
            return buffer[0].raw;
        }

        vtype &get(size_t idx) override
        {
            if (!is_bounded(idx))
            {
                printf("out of bound error\n");
                return buffer[0].raw;
            }
            if(buffer[idx].status){
                return buffer[idx].raw;

            }
            printf("free idx error\n");
            return buffer[0].raw;
        };

        const vtype &get(size_t idx) const override
        {
            if (!is_bounded(idx))
            {
                printf("out of bound error\n");
                return buffer[0].raw;
            }
            return buffer[idx].raw;
        };

        bool status(size_t idx){

            if (!is_bounded(idx))
            {
                printf("out of bound error\n");
                return false;
            }
            return buffer[idx].status;
        }

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
        size_t allocated_element_count() const {return element_count;};

        size_t alloc(){

            for (size_t i = 0; i < array_count; i++)
            {
                if(buffer[i].status == false){
                    buffer[i].status = true;
                    element_count++;
                    return i;
                };
            }
            printf("can't allocate a new entry in alloc_array\n");
            return 0;
        }

        bool free(size_t idx){

            if (!is_bounded(idx))
            {
                printf("out of bound error\n");
                return buffer[0].raw;
            }
            if(!buffer[idx].status){
                printf("error: trying to free an already free array element\n");
                return false;

            }
            element_count--;
            buffer[idx].status = false;
            return true;
        }

        template<typename func>
        bool foreach_entry(func call){
            for (size_t i = 0; i < array_count; i++)
            {
                if(buffer[i].status == true){
                    call(buffer[i].raw);
                };
            }
            return true;
        }
    };

};


#endif // ALLOC_ARRAY_H
