#pragma once
#include <stddef.h>
#include <new>
void *operator new(size_t size);
void *operator new[](size_t size);
void operator delete(void *p) noexcept;
void operator delete[](void *p) noexcept;


#ifdef __ck_host__
inline void *operator new(size_t, void *p) throw() { return p; }
inline void *operator new[](size_t, void *p) throw() { return p; }
inline void operator delete(void *, void *) throw() {};
inline void operator delete[](void *, void *) throw() {};
#endif