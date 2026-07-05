#pragma once
#include <gfx/platform/window.hpp>
#include <stddef.h>

#include "libcore/ds/vec.hpp"
#include "libcore/result.hpp"
#include "libcore/shared.hpp"

namespace wgfx
{
class PlatformApp;

fc::Result<void> initialize_platform();

class PlatformApp
{

public:
    virtual ~PlatformApp() = default;

    fc::Vec<fc::SharedPtr<PlatformWindow>> windows;

    void attach(fc::SharedPtr<PlatformWindow> &window)
    {
        windows.push(window);
    }

    // BackendWindow& create_window(size_t width, size_t height);
};

}; // namespace wgfx
