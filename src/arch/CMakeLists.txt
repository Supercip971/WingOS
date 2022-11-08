
set(ARCH_SPECIFIC "")

if("${ARCH}" STREQUAL "x86_64")
    add_subdirectory(x86)
    add_subdirectory(x86_64)
    set(ARCH_SPECIFIC ${X86_64_ARCH_SPECIFIC})
    set(ARCH_SPECIFIC ${X86_ARCH_SPECIFIC})



elseif("${ARCH}" STREQUAL "i686")
    add_subdirectory(x86)
    set(ARCH_SPECIFIC ${X86_ARCH_SPECIFIC})

endif()


set(ARCH_GENERIC arch.hpp arch.cpp)


add_library(Arch STATIC ${ARCH_GENERIC} ${ARCH_SPECIFIC})