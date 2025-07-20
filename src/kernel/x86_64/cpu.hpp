
#pragma once

#include "arch/x86_64/gdt.hpp"
#include "hw/mem/addr_space.hpp"

#include "kernel/generic/cpu.hpp"
#include "libcore/result.hpp"
namespace arch::amd64
{

constexpr int max_cpu = 512;

class CpuImpl : public Cpu
{
    int _lapic;
    PhysAddr _trampoline_stack;


public:

    Gdt *gdt() ; 
    Gdtr *gdtr() ;
    int lapic() const { return _lapic; };

    CpuImpl(int id, int lapic) : Cpu(id, true), _lapic(lapic) {};
    static CpuImpl *getImpl(int id);
    static CpuImpl *currentImpl();
    static size_t max_processors();

    PhysAddr trampoline_stack() const { return _trampoline_stack; }
    void trampoline_stack(PhysAddr stack) { _trampoline_stack = stack; }

    CpuImpl() : Cpu(-1, false), _lapic(-1) {};
};

void other_cpu_entry();

core::Result<void> cpuContextInit(int id, int lapic);
} // namespace arch::amd64