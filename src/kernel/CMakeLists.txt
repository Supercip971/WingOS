

add_subdirectory(loader)


set(SOURCES main.cpp)

add_subdirectory("${ARCH}")
add_executable(Kernel ${SOURCES} ${LOADER_SOURCES} ${ARCH_SOURCES})
target_link_libraries(Kernel PRIVATE Libc)
if("${ARCH}" STREQUAL "x86_64")

    set(LINKER_SCRIPT  "${PROJECT_SOURCE_DIR}/meta/build/link/${ARCH}/kernel.ld")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -T ${LINKER_SCRIPT} -ffreestanding ")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffreestanding")
    set_target_properties(Kernel PROPERTIES
    LINK_FLAGS "-nostdlib -ffreestanding -nostdlib++ -fno-rtti -fno-stack-protector -e 0")

    add_compile_options(-mcmodel=large -mno-red-zone)
    add_compile_options(-fno-exceptions -fno-rtti)
    add_compile_options(-fno-stack-protector)
    add_compile_options(-fno-pic -fno-pie)
    add_compile_options(-fno-asynchronous-unwind-tables)
    add_compile_options(-fno-lto)
    add_compile_options(-fno-pie)
    add_compile_options(-fno-pic)
    add_compile_options(-mno-mmx)
    add_compile_options(-mno-80387)
    
    
    add_compile_options(-mno-avx)
    add_compile_options(-mno-sse)
    add_compile_options(-mno-sse2)
    add_compile_options(-mno-sse3)
    add_compile_options(-ffreestanding -fbuiltin -nostdlib -nostdlib++)
    

    target_link_libraries(Kernel PRIVATE Arch)

endif()
target_link_libraries(Kernel PRIVATE Mcx)
target_link_libraries(Kernel PRIVATE Core)





install(FILES "${CMAKE_CURRENT_BINARY_DIR}/Kernel" DESTINATION boot)
