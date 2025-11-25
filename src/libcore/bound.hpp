#pragma once 


#ifdef __cplusplus

#ifdef __clang__
#define bounded$ [[clang::lifetimebound]]
#elif defined(__GNUC__)
#define bounded$ 
#else
#define bounded$
#endif 
#endif