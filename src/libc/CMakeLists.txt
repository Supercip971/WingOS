
set(LIBC_SOURCE 
 string.h
 string.c
 )


add_library(Libc STATIC ${LIBC_SOURCE})