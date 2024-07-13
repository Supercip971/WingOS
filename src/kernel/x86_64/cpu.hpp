
#pragma once

#include "kernel/generic/cpu.hpp"
#include "libcore/result.hpp"
namespace arch::amd64
{

class CpuImpl : public Cpu
{
    int _lapic;

public:
    CpuImpl(int id, int lapic) : Cpu(id, true), _lapic(lapic){};
    static CpuImpl *getImpl(int id);
    static CpuImpl *currentImpl();

    CpuImpl() : Cpu(-1, false), _lapic(-1){};
};

core::Result<void> cpuContextInit(int id, int lapic);
} // namespace arch::amd64