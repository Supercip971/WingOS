
set(ARCH_SPECIFIC "")

if("${ARCH}" STREQUAL "x86_64")
    add_subdirectory(x86)
    add_subdirectory(x86_64)
    set(ARCH_SPECIFIC ${X86_64_ARCH_SPECIFIC} ${X86_ARCH_SPECIFIC})
    set(ARCH_SPECIFIC ${X86_ARCH_SPECIFIC})



elseif("${ARCH}" STREQUAL "i686")
    add_subdirectory(x86)
    set(ARCH_SPECIFIC ${X86_ARCH_SPECIFIC})

endif()


set(ARCH_GENERIC arch.hpp arch.cpp)

set(ARCH_SRC ${ARCH_GENERIC} ${ARCH_SPECIFIC})

add_library(Arch STATIC ${ARCH_SRC})

list(TRANSFORM ARCH_SRC PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/")
set(ARCH_SOURCES ${ARCH_SRC} PARENT_SCOPE)

