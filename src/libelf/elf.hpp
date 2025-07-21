#pragma once 

#include <stdint.h>
#include <sys/types.h>
#include "hw/mem/addr_space.hpp"
#include "libcore/enum-op.hpp"
#include "libcore/result.hpp"


enum  class ElfClass : uint16_t
{
    CLASS_INVALID = 0,
    CLASS_32 = 1,
    CLASS_64 = 2,
};

enum class ElfDataEncoding : uint16_t
{
    INVALID = 0,
    ENC_LITTLE_ENDIAN = 1,
    ENC_BIG_ENDIAN = 2,
};

enum class ElfType : uint16_t
{
    NONE = 0,
    RELOCATABLE = 1,
    EXECUTABLE = 2,
    DYNAMIC = 3,
    CORE = 4,
};

enum class ElfProgramHeaderType : uint32_t
{
    HEADER_NULL = 0,
    HEADER_LOAD = 1,
    HEADER_DYNAMIC = 2,
    HEADER_INTERPRET = 3,
    HEADER_NOTE = 4,
};

enum class ElfProgramHeaderFlags : uint16_t 
{
    EXECUTABLE = 1 << 0,
    WRITABLE   = 1 << 1,
    READABLE   = 1 << 2,
};

ENUM_OP$(ElfProgramHeaderFlags);
ENUM_OP$(ElfProgramHeaderType);
ENUM_OP$(ElfType);
ENUM_OP$(ElfClass);






struct __attribute__((packed)) Elf64Header
{
    uint8_t magic[4]; // magic number
    uint8_t elf_class; // 1: 32-bit, 2: 64-bit
    uint8_t ordering; // 1: little-endian, 2: big-endian
    uint8_t version; // version of ELF format
    uint8_t os_abi; // OS ABI
    uint8_t abi_version; // ABI version

    uint8_t _padding[7]; // padding bytes

    uint16_t type; // file type (e.g., executable, shared object)
    uint16_t machine; // target machine architecture
    uint32_t version2; // version of the ELF file format

    uintptr_t entry_point; // entry point address
   
    uint64_t program_header_offset; // offset to program header table
    uint64_t section_header_offset; // offset to section header table

    uint32_t flags; // processor-specific flags
    
    uint16_t header_size; // size of this header
    
    uint16_t program_header_size; // size of program header entry
    uint16_t program_header_count; // number of entries in program header table
    
    uint16_t section_header_size; // size of section header entry
    uint16_t section_header_count; // number of entries in section header table
    
    uint16_t section_name_index; // index of the section name string table
};


struct __attribute__((packed)) Elf64ProgramHeader
{
    uint32_t type; // type of segment (e.g., loadable, dynamic)
    uint32_t flags; // segment flags (e.g., executable, writable)
    
    uint64_t file_offset; // offset in file where segment starts
    uintptr_t virt_addr; // virtual address in memory where segment is loaded
    uintptr_t phys_addr; // physical address (if applicable)
    
    uint64_t file_size; // size of segment in file
    uint64_t mem_size; // size of segment in memory
    
    uint64_t align; // alignment of segment
};

struct  __attribute__((packed)) Elf64SectionHeader{
    uint32_t name; // section name (index into string table)
    uint32_t type; // section type
    uint64_t flags; // section flags
    
    uintptr_t virt_addr; // virtual address in memory
    uint64_t file_offset; // offset in file
    uint64_t file_size; // size of section
    uint32_t link; // index of associated section
    uint32_t info; // extra information
    uint64_t addralign; // alignment of section
    uint64_t entsize; // size of entries if section holds a table
};

namespace elf 
{
    class ElfLoader 
    {
        VirtRange _range;
        Elf64Header _header;
        
        public: 

        VirtRange range() const
        {
            return _range;
        }
        
        PhysRange physical_range() const
        {
            return PhysRange(toPhys(_range.start()), toPhys(_range.end()));
        }
        core::Result<void> verify()
        {
            if (_header.magic[0] != 0x7f || _header.magic[1] != 'E' ||
                _header.magic[2] != 'L' || _header.magic[3] != 'F')
            {
                return "invalid ELF magic number";
            }

            if (_header.elf_class != core::underlying_value( ElfClass::CLASS_64))
            {
                return "unsupported ELF class";
            }

            if (_header.ordering != core::underlying_value( ElfDataEncoding::ENC_LITTLE_ENDIAN))
            {
                return "unsupported data encoding";
            }

            if (_header.type == (uint16_t)ElfType::NONE)
            {
                return "invalid ELF type";
            }

            return {};
        }

        static core::Result<ElfLoader> load(VirtRange range)
        {
            if (range.len() < sizeof(Elf64Header))
            {
                return "range too small to contain ELF header";
            }

            ElfLoader loader;
            loader._range = range;
            loader._header = *range.start().as<Elf64Header>();

            if (loader._header.magic[0] != 0x7f || loader._header.magic[1] != 'E' ||
                loader._header.magic[2] != 'L' || loader._header.magic[3] != 'F')
            {
                return "invalid ELF magic number";
            }

            try$(loader.verify());

            return loader;
        }

        size_t section_count() const
        {
            return this->_header.section_header_count;
        }

        size_t program_count() const
        {
            return this->_header.program_header_count;
        }


        VirtAddr entry_point() const
        {
            return this->_header.entry_point;
        }

        core::Result<Elf64SectionHeader> section_header(size_t index) const
        {
            if (index >= this->section_count())
            {
                return "index out of range";
            }
            return *(Elf64SectionHeader *)((uintptr_t)this->_range.start() + this->_header.section_header_offset + index * this->_header.section_header_size);
        }

        core::Result<Elf64ProgramHeader> program_header(size_t index) const
        {
            if (index >= this->section_count())
            {
                return "index out of range";
            }
            return *(Elf64ProgramHeader *)((uintptr_t)this->_range.start() + this->_header.program_header_offset + index * this->_header.program_header_size);
        }


    };
}
