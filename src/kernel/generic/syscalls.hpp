#pragma once 
#include <stdint.h>
#include "libcore/result.hpp"

#include <wingos-headers/syscalls.h>


core::Result<size_t> syscall_handle(SyscallInterface syscall);

