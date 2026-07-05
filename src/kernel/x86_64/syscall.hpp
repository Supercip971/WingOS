#pragma once

#include "libcore/result.hpp"

namespace arch::amd64
{
fc::Result<void> syscall_init_for_current_cpu();

}
