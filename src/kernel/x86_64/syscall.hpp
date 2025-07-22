#pragma once 



#include "libcore/result.hpp"
namespace arch::amd64 
{
    core::Result<void> syscall_init_for_current_cpu();

}