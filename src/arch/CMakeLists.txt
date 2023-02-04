
set(ARCH_SPECIFIC "")

if("${ARCH}" STREQUAL "x86_64")
    add_subdirectory(x86)
    add_subdirectory(x86_64)
    set(ARCH_SPECIFIC ${X86_64_ARCH_SPECIFIC} ${X86_ARCH_SPECIFIC})



elseif("${ARCH}" STREQUAL "i686")
    add_subdirectory(x86)
    set(ARCH_SPECIFIC ${X86_ARCH_SPECIFIC})

endif()


set(ARCH_GENERIC arch.hpp arch.cpp)

set(ARCH_SRC ${ARCH_GENERIC} ${ARCH_SPECIFIC})
print_all_variables()
add_library(Arch STATIC ${ARCH_SRC})

target_compile_options(Arch PUBLIC $<$<COMPILE_LANGUAGE:ASM_NASM>:
                                        -f elf64>)


target_compile_options(Arch PUBLIC $<$<COMPILE_LANGUAGE:CXX,C>:
                                        -g -Og
                                        -fno-exceptions -fno-rtti
                                        -fno-stack-protector
                                        -fno-pic -fno-pie
                                        -fno-asynchronous-unwind-tables
                                        -fno-lto
                                        -fno-pie
                                        -fno-pic
                                        -mno-mmx
                                        -mno-80387
                                        -mno-avx
                                        -mno-sse
                                        -mno-sse2
                                        -mno-sse3
                                        -ffreestanding -fbuiltin -nostdlib -nostdlib++>)

list(TRANSFORM ARCH_SRC PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/")
set(ARCH_SOURCES ${ARCH_SRC} PARENT_SCOPE)

