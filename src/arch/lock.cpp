// not used for the moment
#include <arch/lock.h>
#include <logging.h>

extern "C" void something_is_bad_i_want_to_die_higher_level(lock_type *the_bad_guy)
{
    the_bad_guy->data = 0;
    log("lock", LOG_ERROR) << "lock : " << the_bad_guy->file << " line : " << the_bad_guy->line << "take too long time to unlock";
}
