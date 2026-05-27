#pragma once

#include "libcore/str_writer.hpp"

#include "libcore/ds/umap.hpp"
#include "libcore/result.hpp"
namespace fc
{

template <typename T>
concept AppRessourceProvider = requires(T *app, const T *appc) {
    {
        app->get_default()
    } -> core::IsConvertibleTo<typename T::AppRessource *>;
    {
        appc->get_default()
    } -> core::IsConvertibleTo<typename T::AppRessource const *>;
    {
        app->load(core::WStr())
    } -> core::IsConvertibleToResult<void>;
    {
        app->find(core::WStr())
    } -> core::IsConvertibleTo<typename T::AppRessource &>;
    {
        app->find(core::WStr())
    } -> core::IsConvertibleTo<typename T::AppRessource const &>;
    {
        app->raw()
    } -> core::IsConvertibleTo<core::UMap<core::WStr, typename T::AppRessource>>;
    {
        T::the()
    } -> core::IsConvertibleTo<T &>;
    {
        app->provide(core::WStr(), new typename T::AppRessource())
    } -> core::IsConvertibleToResult<void>;
};

} // namespace fc
