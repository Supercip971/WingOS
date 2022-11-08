
set(LIBCORE_SOURCES core.hpp core.cpp str.hpp
    io/reader.hpp 
    io/writer.hpp
    io/seekable.hpp
    io/io.hpp 
    io/void.hpp
    type/trait.hpp
    type-utils.hpp
    enum-op.hpp
    optional.hpp
    result.hpp 
    fmt/log.hpp 
    fmt/log.cpp)

add_library(Core STATIC ${LIBCORE_SOURCES})

list(TRANSFORM LIBCORE_SOURCES PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/")
set(LIBCORE_SOURCES ${LIBCORE_SOURCES} PARENT_SCOPE)


