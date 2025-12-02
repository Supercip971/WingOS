#pragma once 


#include "libcore/result.hpp"
#include "libcore/alloc/alloc.hpp"
namespace arch::x86_64 
{

    class SimdContext 
    {


        bool _use_xsave = false;
        size_t _data_size = 0; 
        void* _real_data;
        alignas(16) uint8_t* _data; // space for fxsave area

    public:

        void save(); 

        void load() const; 

        static core::Result<SimdContext> create();

        static core::Result<void> initialize_cpu();

        void release()
        {
            if (_real_data)
            {
                core::mem_free(_real_data);
                _real_data = nullptr;
                _data = nullptr;
            }
        }
    };
};