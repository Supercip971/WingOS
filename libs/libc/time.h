#ifndef TIME_H
#define TIME_H

typedef long int time_t;

struct timespec
{
    time_t tv_sec; /* time in seconds */
    long tv_nsec;  /* time in nanoseconds */
};
#endif
