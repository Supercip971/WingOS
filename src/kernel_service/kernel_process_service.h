#pragma once

#include <stdint.h>

struct raw_process_request
{
    uint64_t m[512];
};
struct get_process_pid
{
    char process_name[128];
};

struct set_current_process_as_service
{
    bool is_ors;
    char service_name[128];
};

struct get_process_buffer
{
    int buffer_type;
    int pid_target;
    bool get_buffer_length; // if true just give the current std length else give the readed length
    uint8_t *target_buffer;
    uint64_t length_to_read;
    uint64_t where;
};

enum process_request_id
{
    GET_PROCESS_PID = 0,
    SET_CURRENT_PROCESS_AS_SERVICE = 1,
    GET_PROCESS_BUFFER = 2
};

struct process_request
{
    uint16_t type;
    union
    {
        raw_process_request rpr;
        get_process_pid gpp;
        set_current_process_as_service scpas;
        get_process_buffer gpb;
    };
} __attribute__((packed));

// so how kernel_process_service work ?
// you can only get process pid threw it, if only the copy size is under sizeof(kernel_process_service_request) = 4k (4096)
void kernel_process_service();
