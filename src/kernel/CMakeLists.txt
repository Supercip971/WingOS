

add_subdirectory(loader)


set(SOURCES main.cpp)

add_subdirectory("${ARCH}")

# We do this, because of the fact that we need to control the compilation of the library.

add_executable(Kernel ${SOURCES} ${LOADER_SOURCES} ${KERNEL_ARCH_SOURCES} ${ARCH_SOURCES} ${MCX_SOURCES} ${LIBC_SOURCES} ${LIBCORE_SOURCES})
#target_link_libraries(Kernel PRIVATE Libc)
if("${ARCH}" STREQUAL "x86_64")

    set(LINKER_SCRIPT  "${PROJECT_SOURCE_DIR}/meta/build/link/${ARCH}/kernel.ld")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -T ${LINKER_SCRIPT} -ffreestanding ")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffreestanding")
    set_target_properties(Kernel PROPERTIES
    LINK_FLAGS "-nostdlib -ffreestanding -nostdlib++ -fno-rtti -fno-stack-protector -e 0")

    target_compile_options(Kernel PUBLIC $<$<COMPILE_LANGUAGE:CXX,C>:
                                        -g -Og -mcmodel=kernel -mno-red-zone
                                        -fno-exceptions -fno-rtti
                                        -fno-stack-protector
                                        -fno-pic -fno-pie
                                        -fno-asynchronous-unwind-tables
                                        -fno-lto
                                        -mno-mmx
                                        -mno-80387
                                        -mno-avx
                                        -mno-sse
                                        -mno-sse2
                                        -mno-sse3
                                        -ffreestanding -fno-builtin -nostdlib -nostdlib++>)
    
    target_compile_options(Kernel PUBLIC $<$<COMPILE_LANGUAGE:ASM_NASM>:
                                        -f elf64>)


    #target_link_libraries(Kernel PRIVATE Arch)
endif()
#target_link_libraries(Kernel PRIVATE Mcx)
#target_link_libraries(Kernel PRIVATE Core)





install(FILES "${CMAKE_CURRENT_BINARY_DIR}/Kernel" DESTINATION boot)
