#pragma once 

#include "kernel/generic/cpu_tree.hpp"
#include "kernel/generic/cpu.hpp"

core::Result<CpuTreeNode*> initialize_cpu_tree();

core::Result<CpuTreeNode*> fallback_use_guessed();
