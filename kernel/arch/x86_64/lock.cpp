// not used for the moment
#include <com.h>
#include <kernel.h>
#include <lock.h>
#include <logging.h>

extern lock_type log_locker;
ASM_FUNCTION void something_is_bad_i_want_to_die_higher_level(lock_type *the_bad_guy)
{
    log_locker.data = 0;
}
