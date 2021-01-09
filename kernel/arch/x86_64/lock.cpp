// not used for the moment
#include <com.h>
#include <kernel.h>
#include <lock.h>
#include <logging.h>

extern lock_type locker_print;
extern lock_type print_locker;
ASM_FUNCTION void something_is_bad_i_want_to_die_higher_level(lock_type *the_bad_guy)
{
    print_locker.data = 0;
    locker_print.data = 0;
    log("lock", LOG_ERROR) << "lock : " << the_bad_guy->file << " line : " << the_bad_guy->line << "take long time to unlock";
}
