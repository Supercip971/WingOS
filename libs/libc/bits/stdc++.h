
#ifndef STD_CPP_H
#define STD_CPP_H
#include <stddef.h>
typedef decltype(alignof(int)) align_value;
namespace std
{

    enum class align_val_t : align_value
    {
    };

}
void *operator new(size_t size);
void *operator new[](size_t size);
void *operator new(size_t size, std::align_val_t align);
void *operator new[](size_t size, std::align_val_t align);
void operator delete(void *p);
void operator delete[](void *p);
void operator delete(void *p);
void operator delete[](void *p);

inline void *operator new(size_t, void *p) throw() { return p; }
inline void *operator new[](size_t, void *p) throw() { return p; }
inline void operator delete(void *, void *) throw(){};
inline void operator delete[](void *, void *) throw(){};
#endif
