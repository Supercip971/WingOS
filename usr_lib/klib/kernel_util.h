#include <stdint.h>

namespace sys
{
    int write_console(const char *raw_data, int length);

    int write_kconsole(const char *raw_data, int length);
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
    enum process_request_id
    {
        GET_PROCESS_PID = 0,
        SET_CURRENT_PROCESS_AS_SERVICE = 1
    };

    struct process_request
    {
        uint16_t type;
        union
        {
            raw_process_request rpr;
            get_process_pid gpp;
            set_current_process_as_service scpas;
        };
    } __attribute__((packed));

    uint64_t get_process_pid(const char *process_name);
    inline void switch_process()
    {
        asm volatile("int 100");
    }
    void set_current_process_as_a_service(const char *service_name, bool is_request_only);
} // namespace sys
