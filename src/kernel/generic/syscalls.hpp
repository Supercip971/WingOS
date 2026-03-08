#pragma once
#include <stdint.h>
#include <wingos-headers/syscalls.h>

#include "kernel/generic/task.hpp"
#include "libcore/result.hpp"

core::Result<size_t> syscall_handle(SyscallInterface syscall, kernel::Task* caller);
