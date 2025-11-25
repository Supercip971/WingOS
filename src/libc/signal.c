
#include "signal.h"
int sigemptyset(sigset_t *__sigset)
{
    if(!__sigset)
        return -1;
    *__sigset = (sigset_t){};
    return 0;
}

int kill(pid_t pid, int sig)
{
    // fixme: implement kill
    (void)pid;
    (void)sig; 
    return 0;

}
int sigprocmask(int how, const sigset_t *set, sigset_t *oset)
{
    if(oset)
    {
        *oset = (sigset_t){};
    }
    (void)how;
    (void)set;
    return 0;
}
