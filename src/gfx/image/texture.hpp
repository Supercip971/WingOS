#pragma once
#include <gfx/image/image.hpp>

#include "gfx/color.hpp"
#include "libcore/optional.hpp"
#include "libcore/shared.hpp"

namespace wgfx
{

enum FilterKind
{
    NEAREST,
    LINEAR,
};

class Texture
{

    core::SharedPtr<Image01> _source;

    core::Optional<ImageRGBA8> _image_rgba8;
    FilterKind _filter_kind = FilterKind::NEAREST;

public:
    Texture() = default;
    Texture(const Texture &) = delete;
    Texture &operator=(const Texture &) = delete;

    Texture(const core::SharedPtr<Image01> &source) : _source(source) {}

    Texture(Texture &&) = default;
    Texture &operator=(Texture &&) = default;

    FilterKind filter_kind() const { return _filter_kind; }

    auto query(float x, float y) const
    {
        float x0 = (x * _source->width());
        float y0 = (y * _source->height());

        if (x0 == _source->width() || y0 == _source->height())
            return Rgba01(0, 0, 0, 0);

        long x0l = static_cast<long>(x0);
        long y0l = static_cast<long>(y0);
        return _source->data()[y0l * _source->width() + x0l];
    }

    auto &query_image_rgba8() const { return _image_rgba8; }

    void compute_image_rgba8()
    {
        if (!_image_rgba8.has_value())
        {
            _image_rgba8 = (_source->copy<Rgba8>());
        }
    }

    auto &image() const { return _source; }
};
} // namespace wgfx
