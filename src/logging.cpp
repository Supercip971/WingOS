#include <com.h>
#include <logging.h>
logging::logging()
{
}
void logging ::set_log_type(const char *data, log_state log_state)
{
    if (log_state == 0)
    {
        printf("\n\033[36m [ %s ] :\033[0m ", data);
    }
    else if (log_state == 1)
    {

        printf("\n\n\033[1m\033[32m [ %s ] :\033[0m ", data);
    }
    else if (log_state == 2)
    {
        printf("\n\033[31;1m [ %s ] :\033[0m ", data);
    }
    else if (log_state == LOG_WARNING)
    {
        printf("\n\033[31m [ %s ] :", data);
    }
    else
    {
        printf("\n\033[4m\033[1m\033[31m [ FATAL ] [ %s ] : \033[0m", data);
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
