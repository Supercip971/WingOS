#pragma once
#include <stddef.h>

#include "libcore/ds/vec.hpp"
#include "libcore/fmt/log.hpp"
#include <gfx/platform/window.hpp>
#include "libcore/shared.hpp"
#include "libcore/type-utils.hpp"

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

}; // namespace gfx
