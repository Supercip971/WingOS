#include <com.h>
#include <device/local_data.h>
#include <logging.h>
logging::logging()
{
}
void logging ::set_log_type(const char *data, log_state log_state)
{
    uint64_t pid = 0;
    uint64_t current_cpu = 0;
    if (get_current_cpu()->current_process != nullptr)
    {
        pid = get_current_cpu()->current_process->pid;
        current_cpu = apic::the()->get_current_processor_id();
    }

    if (log_state == LOG_INFO)
    {
        printf("\n\033[36m[C%x/T%x] [%s] :\033[0m ", current_cpu, pid, data);
    }
    else if (log_state == LOG_DEBUG)
    {

        printf("\n\n\033[1m\033[32m[C%x/T%x] [%s] :\033[0m ", current_cpu, pid, data);
    }
    else if (log_state == LOG_ERROR)
    {
        printf("\n\033[31;1m[C%x/T%x] [%s] :\033[0m ", current_cpu, pid, data);
    }
    else if (log_state == LOG_WARNING)
    {
        printf("\n\033[31m[C%x/T%x] [%s] :", current_cpu, pid, data);
    }
    else
    {
        printf("\n\033[4m\033[1m\033[31m[C%x/T%x][%s][ FATAL ]: \033[0m", current_cpu, pid, data);
    }
}
logging logging ::operator<<(const char *string)
{
    printf(string);
    return *this;
}
logging logging ::operator<<(uint64_t address)
{
    printf(" %x ", address);
    return *this;
}
logging main_logging_system;
logging log(const char *data, log_state color_mode)
{

    main_logging_system.set_log_type(data, color_mode);
    return main_logging_system;
}
