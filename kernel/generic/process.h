#pragma once
#include <arch.h>
#include <backtrace.h>
#include <bitmap.h>
#include <filesystem/userspace_fs.h>
#include <process_context.h>
#include <stdint.h>
#include <utils/config.h>
#include <utils/math.h>
#define SLEEP_ALWAYS ((uint64_t)-1)

#define CURRENT_CPU ((uint64_t)-1)
#define AUTO_SELECT_CPU ((uint64_t)-2)

enum process_state
{
    PROCESS_AVAILABLE = 0,
    PROCESS_RUNNING = 1,
    PROCESS_WAITING = 2,
    PROCESS_CRASH = 3,
    PROCESS_NOT_STARTED = 4,
    PROCESS_SHOULD_BE_DEAD = 5
};

enum process_buffer_type
{
    STDOUT = 0,
    STDIN = 1, // not supported
    STDERR = 2
};

struct process_buffer
{
    uint8_t *data;
    uint64_t length;
    uint64_t allocated_length;
    int type;
} __attribute__((packed));
bool add_process_buffer(process_buffer *buf, uint64_t data_length, uint8_t *raw);
struct process_message
{
    uint8_t message_id;
    uint64_t from_pid;
    uint64_t to_pid;
    uint64_t content_address; // NOTE IT COPY THE CONTENT
    uint64_t content_length;
    uint64_t response;
    bool has_been_readed;
    bool entry_free_to_use;
} __attribute__((packed));
#define MAX_PROCESS_MESSAGE_QUEUE 128
class process
{
    arch_process_data arch_info;
    process_state current_process_state;
    uint64_t upid = 0;
    uint64_t kpid = 0;
    char process_name[128];
    uint64_t entry_point = 0x0;
    uint8_t processor_target;
    bool is_ORS = false;
    uint64_t last_message_used;
    bool should_be_active = false; // used with ORS
    bool user;
    uint8_t *global_process_memory;
    uint64_t global_process_memory_length;
    uint64_t sleeping = 0; // 0 = running | 1[..]infinity = sleeping | SLEEP_ALWAYS = always sleep
    bool is_on_interrupt_process = false;
    uint8_t interrupt_handle_list[8]; // max 8 interrupt per process
    backtrace process_backtrace;
    per_process_userspace_fs ufs;

    process_message *msg_list;

public:
    process(size_t kernel_pid) : current_process_state(PROCESS_AVAILABLE), upid(-1), kpid(kernel_pid){

                                                                                     };

    per_process_userspace_fs &get_ufs() { return ufs; };
    process(const char *name, uint64_t kernel_pid, uint64_t unique_pid, uintptr_t process_entry_point, bool is_user) : upid(unique_pid),
                                                                                                                       kpid(kernel_pid),
                                                                                                                       entry_point(process_entry_point),
                                                                                                                       is_ORS(false),
                                                                                                                       last_message_used(0),
                                                                                                                       user(is_user),
                                                                                                                       sleeping(0)
    {
        memcpy(process_name, name, strlen(name) + 1);
    }

    process(const process &move)
    {

        memcpy(this, &move, sizeof(process));
    }
    process &operator=(const process &move)
    {
        memcpy(this, &move, sizeof(process));
        return *this;
    }
    arch_process_data *get_arch_info() { return &arch_info; };

    void init_global_memory();
    void init_message_system();

    bool is_valid() const
    {
        return kpid != 0;
    }

    bool is_used() const
    {
        return (current_process_state != process_state::PROCESS_AVAILABLE && current_process_state != process_state::PROCESS_CRASH && current_process_state != process_state::PROCESS_SHOULD_BE_DEAD) && is_valid();
    };
    bool is_user() const
    {
        return user;
    }
    bool is_sleeping() const
    {
        return sleeping != 0;
    }
    bool is_runnable(const int cpu) const
    {
        bool can_run_ors = true;
        if (is_on_request() && !should_be_active)
        {
            can_run_ors = false;
        }
        return current_process_state == process_state::PROCESS_WAITING && processor_target == cpu && kpid != 0 && can_run_ors && !is_sleeping();
    }

    void set_cpu(size_t cpuid)
    {
        processor_target = cpuid;
    }
    size_t get_cpu() const { return processor_target; };
    backtrace &get_backtrace() { return process_backtrace; };
    void increase_sleep(size_t tick) { sleeping += tick; };
    void decrease_sleep(size_t tick) { sleeping -= tick; };
    size_t get_sleep_count() const { return sleeping; };

    void kill()
    {
        current_process_state = process_state::PROCESS_SHOULD_BE_DEAD;
        sleeping = -1;
    }

    const char *get_name() { return process_name; };

    size_t get_pid() const { return upid; };
    uint64_t get_kpid() { return kpid; };

    void set_on_request(bool state) { is_ORS = state; };
    void set_state(process_state state) { current_process_state = state; };
    process_state get_state() const { return current_process_state; };
    void set_active(bool state) { should_be_active = state; };

    void create_message_list();
    process_message *read_msg();
    process_message *alloc_message();
    process_message *get_message(uint8_t msg_id) { return &msg_list[msg_id]; };

    bool is_on_request() const { return is_ORS; };
    process_message *create_message(uintptr_t data_addr, uint64_t data_length, uint64_t to_pid);
    static process *from_name(const char *name);
    static process *from_pid(size_t pid);
    static process *current();
    static void set_current(process *target);
    uintptr_t get_global_data_copy(uint64_t offset);
    void *get_global_data(uint64_t offset);
    bool destroy();
    void rename(const char *new_name)
    {

        memcpy(process_name, new_name, utils::max(strlen(process_name), strlen(new_name)));
    }
};

struct message_identifier
{
    uint16_t pid; // process_id
    uint16_t mid; // message_id
} __attribute__((packed));

extern process *process_array;

process *init_process(func entry_point, bool start_direct, const char *name, bool user, uint64_t cpu_target = CURRENT_CPU, int argc = 0, char **argv = nullptr);
void init_multi_process(func start);
void dump_process();
void unlock_process();
void lock_process();

void task_update_switch(process *next);
uintptr_t process_switch_handler(arch_stackframe *isf, bool switch_all);
process_message *send_message(uintptr_t data_addr, uint64_t data_length, const char *to_process);
process_message *send_message_pid(uintptr_t data_addr, uint64_t data_length, uint64_t target_pid);
process_message *read_message();
uint64_t message_response(process_message *indentifier);

// todo : add ORS to user service, for the moment it is only for kernel
void set_on_request_service(bool is_ORS);
void set_on_request_service(bool is_ORS, uint64_t pid);
void on_request_service_update();

void *get_current_process_global_data(uint64_t offset, uint64_t length);

uint64_t get_process_global_data_copy(uint64_t offset, const char *process_name);
void rename_process(const char *name, uint64_t pid);

void sleep(uint64_t count);
void sleep(uint64_t count, uint64_t pid);

void kill(uint64_t pid);
void kill_current();

