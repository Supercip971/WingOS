


set(MCX_SRC mcx.hpp mcx.cpp)

add_library(Mcx STATIC ${MCX_SRC} )

target_link_libraries(Mcx PRIVATE Core)