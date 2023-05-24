#pragma once
#include <libcore/fmt/fmt.hpp>
#include <stdint.h>

#include "libcore/fmt/flags.hpp"
namespace arch::amd64
{
struct __attribute__((packed)) StackFrame
{

    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rbp, rdi, rsi, rdx, rcx, rbx, rax;

    uint64_t interrupt_number, error_code;

    uint64_t rip, cs, rflags, rsp, ss;
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