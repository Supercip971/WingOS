
#include <arch.h>
#include <device/local_data.h>
#include <kernel.h>
#include <logging.h>
utils::lock_type log_locker;
// ------------------------ SHOULD BE REMOVED START ------------------------

// ------------------------ SHOULD BE REMOVED END ------------------------

constexpr const char *log_type_table[5] = {
    "\n\033[36m[%s][%s] :\033[0m ",
    "\n\n\033[1m\033[32m[%s][%s] :\033[0m ",
    "\n\033[31;1m[%s][%s] :\033[0m ",
    "\n\033[4m\033[1m\033[31m[%s][%s][ FATAL ]: \033[0m",
    "\n\033[31m[%s][%s] :",
};

void print_log_str(const char *d, log_state state)
{

    const char *name = "kernel";
    if (process::current() != nullptr)
    {
        name = process::current()->get_name();
    }
    if (state > 4 || state == 3)
    {
        printf(log_type_table[LOG_FATAL], name, d);
        return;
    }
    printf(log_type_table[state], name, d);
}

template <>
void log_print_element<>(void *element)
{
    printf("0x%x", element);
}
template <>
void log_print_element<>(const char *element)
{
    printf(element);
}
template <>
void log_print_element<>(char *element)
{
    printf(element);
}
template <>
void log_print_element<>(char element)
{
    printf("%c", element);
}
template <>
void log_print_element<>(range_str element)
{
    find_device<debug_device>()->echo_out(element.rdata, element.rlength);
}
template <>
void log_print_element<>(uint64_t element)
{
    printf("0x%x", element);
}
template <>
void log_print_element<>(uint32_t element)
{
    printf("0x%x", element);
}
template <>
void log_print_element<>(uint16_t element)
{
    printf("0x%x", element);
}
template <>
void log_print_element<>(uint8_t element)
{
    printf("0x%x", element);
}
template <>
void log_print_element<>(int element)
{
    printf("%i", element);
}
template <>
void log_print_element<>(long element)
{

    printf("%i", element);
}

void slog(const char *msg)
{
    debug_device *dev = find_device<debug_device>();
    dev->echo_out(msg, strlen(msg));
}
void start_log()
{
    log_locker.lock();
    lock_process();
}
void end_log()
{
    unlock_process();
    log_locker.unlock();
}

void logging::set_log_type(const char *data, log_state log_state)
{

    const char *name = "kernel";
    if (process::current() != nullptr)
    {
        name = process::current()->get_name();
    }
    if (log_state > 4 || log_state == 3)
    {
        printf(log_type_table[LOG_FATAL], name, data);
        return;
    }
    printf(log_type_table[log_state], name, data);
}
logging::logging()
{
}
logging::~logging()
{
}
logging main_logging_system;
logging log(const char *data, log_state color_mode)
{
    logging log = logging();
    log.set_log_type(data, color_mode);
    return log;
}

template <>
logging logging::operator<<(void *data)
{

    printf(" %x ", data);
    return *this;
}
template <>
logging logging::operator<<(char *data)
{
    printf(data);
    return *this;
}
template <>
logging logging::operator<<(const char *data)
{
    printf(data);
    return *this;
}

template <>
logging logging::operator<<<int64_t>(int64_t data)
{

    printf(" %x ", data);
    return *this;
}

template <>
logging logging::operator<<<uint64_t>(uint64_t data)
{
    printf(" %x ", data);
    return *this;
}

template <>
logging logging::operator<<<uint32_t>(uint32_t data)
{
    printf(" %x ", data);
    return *this;
}

template <>
logging logging::operator<<<int>(int data)
{
    printf(" %x ", data);
    return *this;
}

template <>
logging logging::operator<<<uint16_t>(uint16_t data)
{
    printf(" %x ", data);
    return *this;
}

template <>
logging logging::operator<<<char>(char data)
{
    printf(" %c ", data);
    return *this;
}
template <>
logging logging::operator<<<unsigned char>(unsigned char data)
{
    printf(" %x ", data);
    return *this;
}
template <>
logging logging::operator<<<range_str>(range_str data)
{
    find_device<debug_device>()->echo_out(data.rdata, data.rlength);
    return *this;
}
