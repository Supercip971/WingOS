#include <stdio.h>
#include <stdlib.h>


#ifndef STBT_ASSERT
#    define STBT_ASSERT(x) ((void)(x))
#endif

#ifndef STBI_ASSERT
#    define STBI_ASSERT(x) ((void)(x))
#endif

#define STBI_NO_THREAD_LOCALS

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.hpp"
