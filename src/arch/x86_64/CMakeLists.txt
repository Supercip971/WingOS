set(X86_64_ARCH_SPECIFIC_SOURCES x86_64.cpp gdt.hpp gdt.cpp idt.cpp idt.hpp interrupts.asm gdt_loading.asm idt.asm interrupts.cpp)


list(TRANSFORM X86_64_ARCH_SPECIFIC_SOURCES PREPEND "x86_64/")

set(X86_64_ARCH_SPECIFIC ${X86_64_ARCH_SPECIFIC_SOURCES} PARENT_SCOPE)

