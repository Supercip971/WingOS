#ifndef RAW_MSG_SYSTEM_H
#define RAW_MSG_SYSTEM_H
#include <stddef.h>
#include <stdint.h>
struct raw_msg_request
{
    bool valid;
    size_t size;
    uint8_t *data;
} __attribute__((packed));

struct raw_msg_call_request
{
    uint64_t argument[5];
    uint64_t return_value;
};

#endif // RAW_MSG_SYSTEM_H
