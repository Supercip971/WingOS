#pragma once



#include <stddef.h>
#include "libcore/type-utils.hpp"
#include <libcore/bound.hpp>

namespace core
{


    template<typename T>

    class SharedPtr
    {

        public:
        struct ControlBlock
        {
            size_t ref_count;

            T data;


            ControlBlock() = default;

            template<typename ...Args>
            ControlBlock (Args ...args)
                : ref_count(0), data(core::forward<Args>(args)...)
            {
            }
        };
        ControlBlock* control_block;


        template<typename ...Args>
        static SharedPtr<T> make(Args ...args)
        {
            SharedPtr<T> ptr = {};
            ptr.control_block = new ControlBlock(core::forward<Args>(args)...);



            ptr.control_block->ref_count = 1;
            return ptr;
        }

        SharedPtr()
         : control_block(nullptr)

         {

         }
        SharedPtr(SharedPtr&& val)
            : control_block(val.control_block)
        {
            val.control_block = nullptr;
        }

        SharedPtr(const SharedPtr& copy)
        {
            control_block = copy.control_block;
            if(control_block)
            {
                control_block->ref_count++;
            }
        }


        T& operator*() bounded$
        {
            return control_block->data;
        }


        T const& operator*() const bounded$
        {
            return control_block->data;
        }

        T* operator->() bounded$
        {
            return &(control_block->data);
        }

        T const* operator->() const bounded$
        {
            return &(control_block->data);
        }



        SharedPtr& operator=(SharedPtr&& other)
        {
            if(this != &other)
            {
                if(control_block && --control_block->ref_count == 0)
                {
                    delete control_block;
                }
                control_block = other.control_block;
                other.control_block = nullptr;
            }
            return *this;
        }


        SharedPtr& operator=(const SharedPtr& other)
        {
            if(this != &other)
            {
                if(control_block && --control_block->ref_count == 0)
                {
                    delete control_block;
                }
                control_block = other.control_block;
                if(control_block)
                {
                    control_block->ref_count++;
                }
            }
            return *this;
        }

        ~SharedPtr()
        {
            if(control_block != nullptr)
            {
                control_block->ref_count--;
                if(control_block->ref_count == 0)
                {
                    delete control_block;
                }

            }

            control_block = nullptr;
        }

    };
}
