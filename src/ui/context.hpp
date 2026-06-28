#pragma once

#include "ui/theme/theme.hpp"

namespace fc
{

class ContextSnapshot
{

public:
    bool operator==(ContextSnapshot const &) const { return true; }
};

class UiContext
{

public:
    UiContext() = default;

    float dpi;
    bool enable_debug_layout = false;
    Theme theme;

    ContextSnapshot snapshot() const { return ContextSnapshot(); }
};

} // namespace fc
