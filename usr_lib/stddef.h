#pragma once

#define NULL 0
typedef unsigned long size_t;

void *operator new(size_t size);
void *operator new[](size_t size);
void operator delete(void *p);
void operator delete[](void *p);
