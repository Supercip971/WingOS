#pragma once
#include <int_value.h>

enum COM_PORT
{
    COM1 = 0x3F8,
    COM2 = 0x2F8,
    COM3 = 0x3E8,
    COM4 = 0x2E8,
};

void com_write_str(const char *buffer);
void com_write_reg(const char *buffer, uint64_t value);

void com_initialize(COM_PORT port);

void com_write_strl(const char *buffer);

void printf(const char *str, ...);
