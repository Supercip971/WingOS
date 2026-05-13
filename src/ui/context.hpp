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

    Theme theme;
    ContextSnapshot snapshot() const { return ContextSnapshot(); }
};

} // namespace fc
