
#ifndef STD_CPP_H
#define STD_CPP_H
#include <stddef.h>
void *operator new(size_t size);
void *operator new[](size_t size);
void operator delete(void *p);
void operator delete[](void *p);
void operator delete(void *p);
void operator delete[](void *p);
#endif
