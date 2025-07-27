#pragma once
#include <stdint.h>
#include <wingos-headers/syscalls.h>

#include "libcore/result.hpp"

core::Result<size_t> syscall_handle(SyscallInterface syscall);
