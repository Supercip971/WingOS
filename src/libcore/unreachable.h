#pragma once


#define NNFAST
#ifdef NNFAST
#define unreachable$() \
    do { \
        __builtin_unreachable(); \
    } while(0)

#else

#define unreachable$() \
    do { \
        asm volatile("ud2"); \
    } while(1)
#endif
