
#include <device/local_data.h>
#include <kernel.h>
#include <logging.h>
void logging::set_log_type(const char *data, log_state log_state)
{
    uint64_t pid = 0;
    uint64_t current_cpu = 0;
    if (get_current_cpu_process() != nullptr)
    {
        pid = get_current_cpu_process()->upid;
        current_cpu = get_current_cpu_id();
    }
    if (log_state > 4 || log_state == 3)
    {
        printf(log_type_table[LOG_FATAL], current_cpu, pid, data);
        return;
    }
    printf(log_type_table[log_state], current_cpu, pid, data);
}
lock_type log_lock;
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
