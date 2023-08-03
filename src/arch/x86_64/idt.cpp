#include "arch/x86_64/idt.hpp"

#include "arch/x86_64/gdt.hpp"
namespace arch::amd64
{

static IDT default_idt;
static IDTRegister default_idt_register;
extern "C" uint64_t __interrupt_vector[256];

static constexpr core::MemView<uint64_t> interrupt_vector = core::MemView<uint64_t>(__interrupt_vector, 256);

IDTRegister *load_default_idt()
{

    default_idt = IDT();
    default_idt.fill(interrupt_vector, Gdt::kernel_code_segment_id * 8);
    default_idt_register = IDTRegister(default_idt._size, reinterpret_cast<uintptr_t>(&default_idt));
    return &default_idt_register;
}

} // namespace arch::amd64