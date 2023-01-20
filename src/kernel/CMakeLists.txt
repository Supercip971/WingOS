

add_subdirectory(loader)


set(SOURCES main.cpp)

add_subdirectory("${ARCH}")

# We do this, because of the fact that we need to control the compilation of the library.

add_executable(Kernel ${SOURCES} ${LOADER_SOURCES} ${ARCH_SOURCES} ${MCX_SOURCES} ${LIBC_SOURCES} ${LIBCORE_SOURCES})
#target_link_libraries(Kernel PRIVATE Libc)
if("${ARCH}" STREQUAL "x86_64")

    set(LINKER_SCRIPT  "${PROJECT_SOURCE_DIR}/meta/build/link/${ARCH}/kernel.ld")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -T ${LINKER_SCRIPT} -ffreestanding ")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffreestanding")
    set_target_properties(Kernel PROPERTIES
    LINK_FLAGS "-nostdlib -ffreestanding -nostdlib++ -fno-rtti -fno-stack-protector -e 0")

    target_compile_options(Kernel PUBLIC -g -Og -mcmodel=large -mno-red-zone)
    target_compile_options(Kernel PUBLIC -fno-exceptions -fno-rtti)
    target_compile_options(Kernel PUBLIC -fno-stack-protector)
    target_compile_options(Kernel PUBLIC -fno-pic -fno-pie)
    target_compile_options(Kernel PUBLIC -fno-asynchronous-unwind-tables)
    target_compile_options(Kernel PUBLIC -fno-lto)
    target_compile_options(Kernel PUBLIC -fno-pie)
    target_compile_options(Kernel PUBLIC -fno-pic)
    target_compile_options(Kernel PUBLIC -mno-mmx)
    target_compile_options(Kernel PUBLIC -mno-80387)
    
    
    target_compile_options(Kernel PUBLIC -mno-avx)
    target_compile_options(Kernel PUBLIC -mno-sse)
    target_compile_options(Kernel PUBLIC -mno-sse2)
    target_compile_options(Kernel PUBLIC -mno-sse3)
    target_compile_options(Kernel PUBLIC -ffreestanding -fbuiltin -nostdlib -nostdlib++)
    

    target_link_libraries(Kernel PRIVATE Arch)

endif()
#target_link_libraries(Kernel PRIVATE Mcx)
#target_link_libraries(Kernel PRIVATE Core)





install(FILES "${CMAKE_CURRENT_BINARY_DIR}/Kernel" DESTINATION boot)
