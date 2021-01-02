#pragma once

#include <stdint.h>

struct raw_process_request
{
    uint64_t m[512];
} __attribute__((packed));
struct get_process_pid
{
    char process_name[128];
} __attribute__((packed));

struct set_current_process_as_service
{
    bool is_ors;
    char service_name[128];
} __attribute__((packed));

struct launch_new_programm
{
    char path[128];
    // not usable for the moment :/
    int argc;
    char **argv; // can't use raw data
} __attribute__((packed));

enum process_request_id
{
    GET_PROCESS_PID = 0,
    SET_CURRENT_PROCESS_AS_SERVICE = 1,
    PROCESS_SLEEP = 3,
    LAUNCH_PROGRAMM = 4,
    GET_CURRENT_PID = 6,
};

struct process_request
{
    uint8_t type;
    union
    {
        raw_process_request rpr;
        get_process_pid gpp;
        set_current_process_as_service scpas;
        uint64_t sleep_counter; // use like that instead of another struct
        launch_new_programm lnp;
        void *gcp;
    };
} __attribute__((packed));

// so how kernel_process_service work ?
// you can only get process pid threw it, if only the copy size is under sizeof(kernel_process_service_request) = 4k (4096)
void kernel_process_service();
