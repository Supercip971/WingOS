#pragma once
#include <stdint.h>

enum file_system_service_request
{
    FILE_OPEN = 1,
    FILE_CLOSE = 2,
    FILE_READ = 3,
    FILE_WRITE = 4,
    GET_FILE_INFO = 5,
    FILE_FIND = 6,
    GET_SUBDIRECTORY_ENTRY = 7
};
struct raw_file_system_request
{
    uint64_t data[128]; // 1ko for file system request, i think we have a lot of space
} __attribute__((packed));
struct file_open_request
{
    uint64_t file_request_id;    // if = 0 create if not re using the entry and close that file (later)
    uint8_t file_directory[255]; // null terminated string
    uint8_t mode[10];
} __attribute__((packed));
struct file_close_request
{
    uint64_t file_request_id;
} __attribute__((packed));
struct file_read
{
    uint8_t *target;
    uint64_t at;
    uint64_t length;
    uint64_t file_request_id;
} __attribute__((packed));

struct file_write
{
    uint8_t *data;
    uint64_t length;
    uint64_t file_request_id;
} __attribute__((packed));
// everything here are not implemented
struct file_find_request
{
    uint8_t file_directory[255]; // null terminated string
};

struct file_information
{
    uint8_t file_type;
    uint64_t access_time;
    uint64_t modify_time;
    uint64_t created_time;
    uint16_t perm;
    uint16_t owner_id;
    uint16_t group_id;
    uint64_t size;
};

struct fill_file_info
{
    uint64_t file_request_id;
    file_information *target;
};
struct file_system_service_protocol
{
    uint16_t request_type;
    union
    {
        raw_file_system_request raw;
        file_open_request open;
        file_close_request close;
        file_read read;
        file_write write; // is not implemented
        fill_file_info info;
    };

} __attribute__((packed));

void file_system_service();
