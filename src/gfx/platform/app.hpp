#pragma once
#include <gfx/platform/window.hpp>
#include <stddef.h>

#include "libcore/ds/vec.hpp"
#include "libcore/result.hpp"
#include "libcore/shared.hpp"

namespace wgfx
{
class PlatformApp;

core::Result<void> initialize_platform();

class PlatformApp
{

public:
    virtual ~PlatformApp() = default;

    core::Vec<core::SharedPtr<PlatformWindow>> windows;

    void attach(core::SharedPtr<PlatformWindow> &window)
    {
        windows.push(window);
    }

    // BackendWindow& create_window(size_t width, size_t height);
};

}; // namespace wgfx
