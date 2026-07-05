#pragma once

#include "libcore/str_writer.hpp"

#include "gfx/image/image.hpp"
#include "gfx/image/texture.hpp"
#include "libcore/ds/umap.hpp"
#include "libcore/fmt/log.hpp"
#include "libcore/path.hpp"
#include "libcore/result.hpp"
#include "libcore/shared.hpp"
#include "libcore/str.hpp"

namespace fc
{
class TextureRepo
{

    fc::UMap<fc::WStr, fc::SharedPtr<wgfx::Texture>> _texture;

public:
    using AppRessource = fc::SharedPtr<wgfx::Texture>;

    fc::Result<void> load(fc::WStr const &name, fc::Str path)
    {

        fmt::log$("loading texture({}): {}", name.view(), path);
        auto fpath = fc::finalizePath(path);
        _texture.insert(
            fc::WStr::copy(name.view()),
            fc::SharedPtr<wgfx::Texture>::make(
                fc::SharedPtr<wgfx::Image01>::make(try$(
                    wgfx::Image01::load_from_file(fpath.view())))));
        _texture[name.view()]->compute_image_rgba8();
        return {};
    }

    fc::SharedPtr<wgfx::Texture> &find(fc::Str const &name)
    {
        return _texture[name];
    }

    fc::SharedPtr<wgfx::Texture> const &find(fc::Str const &name) const
    {
        return _texture[name];
    }

    fc::UMap<fc::WStr, fc::SharedPtr<wgfx::Texture>> &raw()
    {
        return _texture;
    }

    fc::UMap<fc::WStr, fc::SharedPtr<wgfx::Texture>> const &raw() const
    {
        return _texture;
    }

    static TextureRepo &the();
};

} // namespace fc
