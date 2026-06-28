#include <stdio.h>
#include <stdlib.h>

#ifndef STBTT_assert
#    define STBTT_assert(x) ((void)(x))
#endif

#define STBI_NO_THREAD_LOCALS

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.hpp"
