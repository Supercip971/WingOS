

if("${LOADER}" STREQUAL "limine")
    set(LOADER_SOURCES limine/entry.cpp limine/limine.hpp limine/load_ctx.cpp)


list(TRANSFORM LOADER_SOURCES PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/")

set(LOADER_SOURCES ${LOADER_SOURCES} PARENT_SCOPE)
endif()