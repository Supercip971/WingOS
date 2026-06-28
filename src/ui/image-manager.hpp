#pragma once

#include "libcore/str_writer.hpp"

#include "gfx/image/image.hpp"
#include "gfx/image/texture.hpp"
#include "gfx/text/font.hpp"
#include "libcore/ds/umap.hpp"
#include "libcore/path.hpp"
#include "libcore/shared.hpp"

namespace fc
{
class TextureRepo
{

    core::UMap<core::WStr, core::SharedPtr<wgfx::Texture>> _texture;

public:
    using AppRessource = core::SharedPtr<wgfx::Texture>;

    core::Result<void> load(core::WStr const &name, core::Str path)
    {

        fmt::log$("loading texture({}): {}", name.view(), path);
        auto fpath = core::finalizePath(path);
        _texture.insert(
            core::WStr::copy(name.view()),
            core::SharedPtr<wgfx::Texture>::make(
                core::SharedPtr<wgfx::Image01>::make(try$(
                    wgfx::Image01::load_from_file(fpath.view())))));
        _texture[name.view()]->compute_image_rgba8();
        return {};
    }

    core::SharedPtr<wgfx::Texture> &find(core::Str const &name)
    {
        return _texture[name];
    }

    core::SharedPtr<wgfx::Texture> const &find(core::Str const &name) const
    {
        return _texture[name];
    }

    core::UMap<core::WStr, core::SharedPtr<wgfx::Texture>> &raw()
    {
        return _texture;
    }

    core::UMap<core::WStr, core::SharedPtr<wgfx::Texture>> const &raw() const
    {
        return _texture;
    }

    static TextureRepo &the();
};

} // namespace fc
