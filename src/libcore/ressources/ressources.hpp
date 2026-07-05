#pragma once

#include "libcore/str_writer.hpp"

#include "libcore/ds/umap.hpp"
#include "libcore/result.hpp"
#include "libcore/type/trait.hpp"

namespace fc
{

template <typename T>
concept AppRessourceProvider = requires(T *app, const T *appc) {
    {
        app->get_default()
    } -> fc::IsConvertibleTo<typename T::AppRessource *>;
    {
        appc->get_default()
    } -> fc::IsConvertibleTo<typename T::AppRessource const *>;
    {
        app->load(fc::WStr())
    } -> fc::IsConvertibleToResult<void>;
    {
        app->find(fc::WStr())
    } -> fc::IsConvertibleTo<typename T::AppRessource &>;
    {
        app->find(fc::WStr())
    } -> fc::IsConvertibleTo<typename T::AppRessource const &>;
    {
        app->raw()
    } -> fc::IsConvertibleTo<fc::UMap<fc::WStr, typename T::AppRessource>>;
    {
        T::the()
    } -> fc::IsConvertibleTo<T &>;
    {
        app->provide(fc::WStr(), new typename T::AppRessource())
    } -> fc::IsConvertibleToResult<void>;
};

} // namespace fc
