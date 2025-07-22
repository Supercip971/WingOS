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
    fmt::format(target, "  - r15 : {} r14: {} r13: {} \n", value.r15 | flags, value.r14 | flags, value.r13 | flags);
    fmt::format(target, "  - r12 : {} r11: {} r10: {} \n", value.r12 | flags, value.r11 | flags, value.r10 | flags);
    fmt::format(target, "  - r9  : {} r8 : {} \n", value.r9 | flags, value.r8 | flags);
    fmt::format(target, "  - rbp : {} rdi: {} rsi: {} \n", value.rbp | flags, value.rdi | flags, value.rsi | flags);
    fmt::format(target, "  - rdx : {} rcx: {} rbx: {} \n", value.rdx | flags, value.rcx | flags, value.rbx | flags);
    fmt::format(target, "  - rax : {} \n", value.rax | flags);
    fmt::format(target, "  - interrupt n°{} \n", value.interrupt_number | fmt::FMT_CYAN);
    fmt::format(target, "  - error code: {} \n", value.error_code | flags);
    fmt::format(target, "  - ip        : {} \n", value.rip | flags);
    fmt::format(target, "  - cs        : {} \n", value.cs | flags);
    fmt::format(target, "  - rflags    : {} \n", value.rflags | flags);
    fmt::format(target, "  - rsp       : {} \n", value.rsp | flags);
    fmt::format(target, "  - ss        : {} \n", value.ss | flags);
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