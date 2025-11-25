#pragma once

#ifdef __cplusplus
extern "C" {
#endif
#include <sys/types.h>
#include <time.h>



#define S_IFMT 0xF000 // type of file
#define S_IFDIR 0x4000 // directory
#define S_IFCHR 0x2000 // character special
#define S_IFBLK 0x6000 // block special
#define S_IFREG 0x8000 // regular file
#define S_IFIFO 0x1000 // fifo
#define S_IFLNK 0xA000 // symbolic link
#define S_IFSOCK 0xC000 // socket

#define S_ISDIR(mode) (((mode) & S_IFMT) == S_IFDIR)
#define S_ISCHR(mode) (((mode) & S_IFMT) == S_IFCHR)
#define S_ISBLK(mode) (((mode) & S_IFMT) == S_IFBLK)
#define S_ISREG(mode) (((mode) & S_IFMT) == S_IFREG)
#define S_ISFIFO(mode) (((mode) & S_IFMT) == S_IFIFO)
#define S_ISLNK(mode) (((mode) & S_IFMT) == S_IFLNK)
#define S_ISSOCK(mode) (((mode) & S_IFMT) == S_IFSOCK)

#define PATH_MAX 110
struct stat
{

    dev_t st_dev;         // ID of device containing file
    ino_t st_ino;         // file serial number
    mode_t st_mode;       // mode of file (see below)
    nlink_t st_nlink;     // number of links to the file
    uid_t st_uid;         // user ID of file
    gid_t st_gid;         // group ID of file
    dev_t st_rdev;        // device ID (if file is character or block special)
    off_t st_size;        // file size in bytes (if file is a regular file)
    time_t st_atime;      // time of last access
    time_t st_mtime;      // time of last data modification
    time_t st_ctime;      // time of last status change
    blksize_t st_blksize; // a filesystem-specific preferred I/O block size for
                          // this object.  In some filesystem types, this may
                          // vary from file to file
    blkcnt_t st_blocks; // number of blocks allocated for this object
};


int stat(const char * __restrict path, struct stat * __restrict buf);






#ifdef __cplusplus
}
#endif