#pragma once 


#include <stddef.h>
#include "hw/mem/addr_space.hpp"
#include "libcore/ds/vec.hpp"
#include "libcore/lock/lock.hpp"
#include "libcore/result.hpp"
#include "mcx/mcx.hpp"
namespace kernel
{

     enum KasanTags : uint8_t  {
        KASAN_TAG_USERSPACE = 0x00,
        KASAN_TAG_FREE = 0x01,
        KASAN_TAG_ALLOCATED = 0xAB,
        KASAN_TAG_KERNEL = 0xFF,
        KASAN_UNAVAILABLE = 0xFE
    };
    class Kasan 
    {
        
        static constexpr uintptr_t SHADOW_TAG_MASK = 0xFF00000000000000;
        uint8_t* _shadow_memory = nullptr;
        size_t _shadow_memory_size = 0;
        bool _enabled = false;

        template<typename T>
        constexpr static T use_addr_tag(T  addr, KasanTags tag)
        {
            uintptr_t addr_val = reinterpret_cast<uintptr_t>(addr);
            addr_val = (addr_val & ~SHADOW_TAG_MASK) | (static_cast<uintptr_t>(tag) << 56);
            return reinterpret_cast<T>(addr_val);
        }


        core::Lock _lock;
        
        public:
        

        void set_region_tag(VirtRange range,KasanTags tag);

        template<typename T>
        T set_addr_tag(T addr, size_t size, KasanTags tag)
        {
            uintptr_t addr_val = reinterpret_cast<uintptr_t>(addr);

            uintptr_t new_addr_val = (addr_val & ~SHADOW_TAG_MASK) | (static_cast<uintptr_t>(tag) << 56);

            set_region_tag(VirtRange::from_begin_len(VirtAddr{new_addr_val}, size), tag);
            return reinterpret_cast<T>(new_addr_val);
        }

        void preload(mcx::MachineContext* ctx);

        void enable() { _enabled = true; }
        void disable() { _enabled = false; }
        void assert_write_access(uintptr_t addr, size_t size);
        void assert_read_access(uintptr_t addr, size_t size);
        Kasan () = default;
    static         core::Result<void> initialize(size_t memory_size);
        static Kasan &the();


    };
}