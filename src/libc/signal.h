#pragma once 
#include <stdint.h>
#include <sys/types.h>
typedef int sig_atomic_t; 


#define SIG_BLOCK 0
#define SIG_UNBLOCK 1
#define SIG_SETMASK 2
typedef int64_t sigset_t;


int sigemptyset(sigset_t *__sigset);

int kill(pid_t pid, int sig);

int sigprocmask(int how, const sigset_t *set, sigset_t *oset);
