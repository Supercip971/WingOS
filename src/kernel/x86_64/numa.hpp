#pragma once

#include "kernel/generic/cpu_tree.hpp"

#include "libcore/result.hpp"

fc::Result<CpuTreeNode *> initialize_cpu_tree();

fc::Result<CpuTreeNode *> fallback_use_guessed();
