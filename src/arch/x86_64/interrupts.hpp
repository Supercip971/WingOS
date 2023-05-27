
#pragma once
#include <libcore/ds/array.hpp>
#include <libcore/str.hpp>
namespace arch::amd64
{

// table 8-1 (part 8.2 Vectors) amd64 architecture programmer's manual volume 2

static constexpr core::Array interrupts_names{
    core::Str("Divide by zero [#DE]"),
    "Debug [#DB]",
    "Non-maskable interrupt [#NMI]",
    "Breakpoint [#BP]",
    "Overflow [#OF]",
    "Bound range [#BR]",
    "Invalid opcode [#UD]",
    "Device not available [#NM]",
    "Double fault [#DF]",
    "Coprocessor segment overrun [unsupported]",
    "Invalid TSS [#TS]",
    "Segment not present [#NP]",
    "Invalid stack segment [#SS]",
    "General protection fault [#GP]",
    "Page fault [#PF]",
    "Reserved (15)",
    "x87 floating-point exception-pending [#MF]",
    "Alignment check fault [#AC]",
    "Machine check fault [#MC]",
    "SIMD floating-point exception [#XF]",
    "Reserved (20)",
    "Control Protection Exception [#CP]",
    "Reserved (22)",
    "Reserved (23)",
    "Reserved (24)",
    "Reserved (25)",
    "Reserved (26)",
    "Reserved (27)",
    "Hypervisor Injection exception [#HV]",
    "Vmm communicaiton excpetion [#VC]",
    "Security exception [#SX]",
    "Reserved (31)",

};
} // namespace arch::amd64