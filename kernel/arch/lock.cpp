// not used for the moment
#include <arch/lock.h>
#include <com.h>
#include <kernel.h>
#include <logging.h>
ASM_FUNCTION void something_is_bad_i_want_to_die_higher_level(lock_type *the_bad_guy)
{
    if (the_bad_guy->cantforce)
    {
        locker_print.data = 0;
        print_locker.data = 0;
        log("lock", LOG_ERROR) << "lock : " << the_bad_guy->file << " line : " << the_bad_guy->line << "take time to unlock";
        return;
    }
    print_locker.data = 0;
    locker_print.data = 0;
    log("lock", LOG_ERROR) << "lock : " << the_bad_guy->file << " line : " << the_bad_guy->line << "take too long time to unlock | forcing the unlock";

    the_bad_guy->data = 0;
}
