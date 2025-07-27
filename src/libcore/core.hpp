#pragma once
#include <stddef.h>

void *operator new(size_t size);
void *operator new[](size_t size);
void operator delete(void *p) noexcept;
void operator delete[](void *p) noexcept;

constexpr inline void *operator new(size_t, void *p) throw() { return p; }
constexpr inline void *operator new[](size_t, void *p) throw() { return p; }
constexpr inline void operator delete(void *, void *) throw() {};
constexpr inline void operator delete[](void *, void *) throw() {};
