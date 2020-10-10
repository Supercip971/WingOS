#include <stdint.h>

namespace sys {
    int write_console(const char* raw_data, int length);

    uint64_t get_process_pid(const char* process_name);
}
