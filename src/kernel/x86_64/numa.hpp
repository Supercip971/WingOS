#pragma once

#include "kernel/generic/cpu_tree.hpp"

#include "libcore/result.hpp"

core::Result<CpuTreeNode *> initialize_cpu_tree();

core::Result<CpuTreeNode *> fallback_use_guessed();
