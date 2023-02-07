#pragma once 

#include <stdint.h>
#include "libcore/ds/array.hpp"
#include "libcore/enum-op.hpp"
namespace arch::amd64 
{
    struct IDTRegister
    {
        uint16_t _size; 
        uint64_t _addr;
        constexpr IDTRegister() : _size{0}, _addr{0} {
            
        }
        constexpr IDTRegister(unsigned short size, uintptr_t addr) : _size{size} , _addr{addr} {}; 
    } __attribute__((packed));

    struct IDTEntry 
    {
        enum class Type
        {
            TRAP = 0xeF,
            USER = 0x60,
            GATE =  0x8e, 
        };

        
        uint16_t _offset_0_16; 
        uint16_t _code_segment; 
        uint8_t _ist_index;
        uint8_t _attributes; 
        uint16_t _offset_16_32;
        uint32_t _offset_32_64;
        uint32_t _zero;

        constexpr IDTEntry() : 
        _offset_0_16{0}, _code_segment{0}, 
        _ist_index{0}, _attributes{0},
        _offset_16_32{0}, _offset_32_64{0},
        _zero{0}{};

        constexpr IDTEntry(uint64_t offset,uint16_t cs,  uint8_t ist, IDTEntry::Type attribute) : 
            _offset_0_16{static_cast<uint16_t>(offset)},
            _code_segment{cs},
            _ist_index{ist},
            _attributes{static_cast<uint8_t>(attribute)},
            _offset_16_32{static_cast<uint16_t>(offset >> 16)},
            _offset_32_64{static_cast<uint32_t>(offset >> 32)}, 
            _zero{0}{}; 
        
    } __attribute__((packed));
    
    ENUM_OP$(IDTEntry::Type);

    struct IDT 
    {
        static const int _size = 256;
        core::Array<IDTEntry, _size> _entries;

        constexpr IDT() : _entries{} {};

        
        void fill( uintptr_t handler_table_base, int cs) {
            for(int i = 0; i < _size; i++)
            {
                _entries[i] = IDTEntry(reinterpret_cast<uintptr_t>(handler_table_base) + i*8, cs, 0, IDTEntry::Type::TRAP);
            }
        }

    } __attribute__((packed)); 

    IDTRegister* load_default_idt();

    extern "C" void idt_use(IDTRegister* idtr);

    static_assert(sizeof(IDT) == sizeof(IDTEntry)*256, "IDT structure must have a valid size" );    
}