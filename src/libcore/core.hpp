#pragma once
#include <stddef.h>

void *operator new(size_t size);
void *operator new[](size_t size);
void operator delete(void *p) noexcept;
void operator delete[](void *p) noexcept;

inline __attribute__((weak)) void *operator new(size_t, void *p) throw() { return p; }
inline __attribute__((weak)) void *operator new[](size_t, void *p) throw() { return p; }
inline __attribute__((weak)) void operator delete(void *, void *) throw() {};
inline __attribute__((weak)) void operator delete[](void *, void *) throw() {};
