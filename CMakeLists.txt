


cmake_minimum_required(VERSION 3.15...3.24)

project(
  Wingos
  VERSION 1.0
  LANGUAGES CXX C)
enable_language(ASM_NASM)
macro(print_all_variables)
    message(STATUS "print_all_variables------------------------------------------{")
    get_cmake_property(_variableNames VARIABLES)
    foreach (_variableName ${_variableNames})
        message(STATUS "${_variableName}=${${_variableName}}")
    endforeach()
    message(STATUS "print_all_variables------------------------------------------}")
endmacro()
set(ARCH "x86_64")
set(LOADER "limine")
set(CMAKE_STAGING_PREFIX "${CMAKE_BINARY_DIR}/root-path")
set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/root-path")
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/meta/build" ${CMAKE_MODULE_PATH})

set(CMAKE_INCLUDE_PATH "${PROJECT_SOURCE_DIR}/src")
set(BUILD_SHARED_LIBS OFF)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")
add_compile_options("$<$<COMPILE_LANGUAGE:CXX,C>:-fno-exceptions;-fno-rtti;-fno-stack-protector;-Wall;-Wextra;-Werror;-Wvla;-Wshadow;-Og;-g;>")


#add_compile_options(-O3) 


add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-std=c++2b>)




  
include("toolchains/llvm")
include_directories(SYSTEM "${PROJECT_SOURCE_DIR}/src")
file(TO_CMAKE_PATH "${PROJECT_BINARY_DIR}/CMakeLists.txt" LOC_PATH)

if(EXISTS "${LOC_PATH}")
    message(FATAL_ERROR "You cannot build in a source directory (or any directory with a CMakeLists.txt file). Please make a build subdirectory. Feel free to remove CMakeCache.txt and CMakeFiles.")
endif()


if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
endif()

add_subdirectory(src)

add_custom_target(limine-image
    COMMAND ${CMAKE_COMMAND} -E env BUILD_DIR="${CMAKE_BINARY_DIR}" "${PROJECT_SOURCE_DIR}/meta/build/scripts/gen-disk-limine.sh"
    BYPRODUCTS ${CMAKE_BINARY_DIR}/disk.hdd
    USES_TERMINAL
)

add_custom_target(run-qemu
    COMMAND 
        "${CMAKE_COMMAND}" -E env BUILD_DIR="${CMAKE_BINARY_DIR}" "${PROJECT_SOURCE_DIR}/meta/build/scripts/run-qemu.sh"
    USES_TERMINAL
)
add_custom_target(setup-and-run
    COMMAND ${CMAKE_MAKE_PROGRAM} install
    COMMAND ${CMAKE_MAKE_PROGRAM} limine-image
    COMMAND ${CMAKE_MAKE_PROGRAM} run-qemu
    USES_TERMINAL
)