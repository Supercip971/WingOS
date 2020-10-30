#pragma once
#include "liballoc.h"

void *operator new(uint64_t size);
void *operator new[](uint64_t size);
void operator delete(void *p);
void operator delete[](void *p);
