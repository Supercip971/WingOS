#pragma once
#include <stdint.h>

namespace sys
{
    int write_console(const char *raw_data, int length);

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

    uint64_t get_current_pid();
    uint64_t get_process_pid(const char *process_name);
    inline void switch_process()
    {
        asm volatile("int 100");
    }
    void set_current_process_as_a_service(const char *service_name, bool is_request_only);
    void ksleep(uint64_t time); // in ms
    uint64_t start_programm(const char *path);
} // namespace sys
