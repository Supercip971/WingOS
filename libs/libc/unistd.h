#ifndef UNISTD_H
#define UNISTD_H
#include <stdint.h>
#include <time.h>
// FIXME: move it in sys/types.h

typedef long suseconds_t;

unsigned int sleep(unsigned int sec);
suseconds_t usleep(suseconds_t sec);
#endif // UNISTD_H
