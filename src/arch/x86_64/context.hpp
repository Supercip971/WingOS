#pragma once
#include <libcore/fmt/fmt.hpp>
#include <stdint.h>

#include "libcore/fmt/flags.hpp"
namespace arch::amd64
{

struct __attribute__((packed)) SyscallStackFrame
{
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rbp, rdi, rsi, rdx, rcx, rbx, rax;

    uintptr_t rip;
    uintptr_t cs;
    uintptr_t rflags;
    uintptr_t rsp;
    uintptr_t ss;
};

struct __attribute__((packed)) StackFrame
{
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rbp, rdi, rsi, rdx, rcx, rbx, rax;

    uint64_t interrupt_number, error_code;

    uint64_t rip, cs, rflags, rsp, ss;
};

enum RFlagsValues
{
    RFLAGS_CARRY = (1 << 0),
    RFLAGS_ONE = (1 << 1),
    RFLAGS_PARITY = (1 << 2),
    RFLAGS_AUX_CARRY = (1 << 4),
    RFLAGS_ZERO = (1 << 6),
    RFLAGS_SIGN = (1 << 7),
    RFLAGS_TRAP = (1 << 8),

    RFLAGS_INTERRUPT_ENABLE = (1 << 9),
    RFLAGS_DIRECTION = (1 << 10),
    RFLAGS_OVERFLOW = (1 << 11),
    RFLAGS_IOPL = (1 << 12),
    RFLAGS_NT = (1 << 14),
    RFLAGS_RF = (1 << 16),

    RFLAGS_VIRTUAL8086_MODE = (1 << 17),
    RFLAGS_ALIGNMENT_CHECK = (1 << 18),
    RFLAGS_VIRTUAL_INTERRUPT = (1 << 19),
    RFLAGS_VIRTUAL_INTERRUPT_PENDING = (1 << 20),
    RFLAGS_ID = (1 << 21),
};

template <core::IsConvertibleTo<StackFrame const> T, core::Writable Targ>
constexpr core::Result<void> format_v(Targ &target, T value)
{

    // FIXME: add a way to use the same format for all parameters,
    // instead of repeating FMT_HEX | FMT_CYAN for each one of them

#define flags fmt::FMT_HEX | fmt::FMT_CYAN | fmt::FMT_PAD_8BYTES | fmt::FMT_PAD_ZERO
    fmt::format(target, "\n Stackframe: [ \n");
    auto r15_val = value.r15, r14_val = value.r14, r13_val = value.r13;
    fmt::format(target, "  - r15 : {} r14: {} r13: {} \n", r15_val | flags, r14_val | flags, r13_val | flags);
    auto r12_val = value.r12, r11_val = value.r11, r10_val = value.r10;
    fmt::format(target, "  - r12 : {} r11: {} r10: {} \n", r12_val | flags, r11_val | flags, r10_val | flags);
    auto r9_val = value.r9, r8_val = value.r8;
    fmt::format(target, "  - r9  : {} r8 : {} \n", r9_val | flags, r8_val | flags);
    auto rbp_val = value.rbp, rdi_val = value.rdi, rsi_val = value.rsi;
    fmt::format(target, "  - rbp : {} rdi: {} rsi: {} \n", rbp_val | flags, rdi_val | flags, rsi_val | flags);
    auto rdx_val = value.rdx, rcx_val = value.rcx, rbx_val = value.rbx;
    fmt::format(target, "  - rdx : {} rcx: {} rbx: {} \n", rdx_val | flags, rcx_val | flags, rbx_val | flags);
    auto rax_val = value.rax;
    fmt::format(target, "  - rax : {} \n", rax_val | flags);
    auto interrupt_num = value.interrupt_number;
    fmt::format(target, "  - interrupt n°{} \n", interrupt_num | fmt::FMT_CYAN);
    auto error_code = value.error_code;
    fmt::format(target, "  - error code: {} \n", error_code | flags);
    auto rip_val = value.rip;
    fmt::format(target, "  - ip        : {} \n", rip_val | flags);
    auto cs_val = value.cs;
    fmt::format(target, "  - cs        : {} \n", cs_val | flags);
    auto rflags_val = value.rflags;
    fmt::format(target, "  - rflags    : {} \n", rflags_val | flags);
    auto rsp_val = value.rsp;
    fmt::format(target, "  - rsp       : {} \n", rsp_val | flags);
    auto ss_val = value.ss;
    fmt::format(target, "  - ss        : {} \n", ss_val | flags);
    fmt::format(target, "] \n");

#undef flags
    return {};
}

template <core::IsSame<StackFrame const *> T, core::Writable Targ>
constexpr core::Result<void> format_v(Targ &target, T value)
{
    return format_v(target, *value);
}

} // namespace arch::amd64