


set(MCX_SRC mcx.hpp mcx.cpp)

add_library(Mcx STATIC ${MCX_SRC} )
target_link_libraries(Mcx PRIVATE Core)

list(TRANSFORM MCX_SRC PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/")
set(MCX_SOURCES ${MCX_SRC} PARENT_SCOPE)

