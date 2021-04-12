#ifndef SYS_TYPES_H
#define SYS_TYPES_H
#include <stddef.h>
typedef long ssize_t; // signed size_t

typedef long blkcnt_t;    // block count
typedef size_t blksize_t; // block size

typedef blkcnt_t fsblkcnt_t; // file system block count

typedef size_t clock_t; // clock tick
typedef long clockid_t; // clock id

typedef unsigned long id_t; // generic id

typedef int gid_t;           // group id
typedef long pid_t;          // process & process group id
typedef unsigned long uid_t; // user id

typedef long suseconds_t;      // time in microsecond
typedef long time_t;           // time in second
typedef unsigned long timer_t; // timer id

#endif
