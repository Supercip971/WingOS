#pragma once

#include <stdint.h>
struct kernel_process_service_request
{
    uint64_t type = 0;

    union
    {
        uint64_t raw_data[511];
    };
};

// so how kernel_process_service work ?
// you can only get process pid threw it, if only the copy size is under sizeof(kernel_process_service_request) = 4k (4096)
void kernel_process_service();
