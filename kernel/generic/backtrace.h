#ifndef BACKTRACE_H
#define BACKTRACE_H
#include <arch.h>

class backtrace
{
    static const int backtrace_max_entry_count = 32;
    backtrace_entry_type entry[backtrace_max_entry_count];

public:
    backtrace();
    void add_entry(const backtrace_entry_type added_entry);
    void dump_backtrace();
} __attribute__((packed));

#endif // BACKTRACE_H
