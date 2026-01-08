#pragma once

#include <stdint.h>
#include <atomic>

#include "libcore/core.hpp"
#include "libcore/lock/lock.hpp"
#include "libcore/result.hpp"
#include "libcore/type-utils.hpp"
#include "wingos-headers/asset.h"

// Canonical kernel-side Asset + AssetRef definitions.
//
// Goals:
// - Provide a single source of truth for `Asset` and `AssetRef<T>`
// - Avoid circular includes between `space.hpp`, `ipc.hpp`, and other kernel headers
// - Keep this header minimal (no Space / IPC / Task includes)

struct Asset : public core::NoCopy
{
    // Provide an IDENT constant so templates written against typed assets can also be instantiated with `Asset`.
    static constexpr size_t IDENT = (size_t)OBJECT_KIND_UNKNOWN;

    core::Lock lock{};
    std::atomic<size_t> ref_count{0};
    AssetKind kind{AssetKind::OBJECT_KIND_SPACE};

    explicit Asset(AssetKind kind_value)
        : ref_count(0), kind(kind_value)
    {
    }

    virtual ~Asset() = default;

    virtual void destroy()
    {
        delete this;
    }

    template <typename T>
    T *casted()
    {
        return static_cast<T *>(this);
    }

    template <typename T>
    T const *casted() const
    {
        return static_cast<T const *>(this);
    }

    template <typename T>
    core::Result<T *> as()
    {
        if (kind != T::IDENT)
        {
            return "invalid cast";
        }
        return static_cast<T *>(this);
    }

    template <typename T>
    core::Result<T const *> as() const
    {
        if (kind != T::IDENT)
        {
            return "invalid cast";
        }
        return static_cast<T const *>(this);
    }

    // Implemented out-of-line (see `asset.cpp`) to avoid duplicate definitions across TUs.
    static void own(Asset *asset);
    static void deref(Asset *asset);
    static void release(Asset *asset);
};

template <typename T = Asset>
struct AssetRef
{
    T *asset{nullptr};
    uint64_t handle{(uint64_t)-1};
    bool write{true};
    bool read{true};
    bool share{true};

    AssetRef() = default;


    template<typename T2>
    AssetRef(AssetRef<T2> const& other)
    {
        asset = reinterpret_cast<T *>(other.asset);
        handle = other.handle;
        write = other.write;
        read = other.read;
        share = other.share;

        if(asset != nullptr)
        {
            Asset::own(reinterpret_cast<Asset *>(asset));
        }
    }

    AssetRef(T *asset_value, uint64_t handle_value)
        : asset(asset_value), handle(handle_value), write(true), read(true), share(true)
    {
        Asset::own(reinterpret_cast<Asset *>(asset_value));
    }

    AssetRef(T *asset_value, uint64_t handle_value, bool write_value, bool read_value, bool share_value)
        : asset(asset_value), handle(handle_value), write(write_value), read(read_value), share(share_value)
    {
        Asset::own(reinterpret_cast<Asset *>(asset_value));
    }

    AssetRef(AssetRef const &other)
        : asset(other.asset),
          handle(other.handle),
          write(other.write),
          read(other.read),
          share(other.share)
    {
        if(asset != nullptr)
        {
            Asset::own(reinterpret_cast<Asset *>(asset));
        }
    }

    AssetRef(AssetRef &&other)
        : asset(other.asset),
          handle(other.handle),
          write(other.write),
          read(other.read),
          share(other.share)
    {
        other.asset = nullptr;
        other.handle = 0;
        other.write = false;
        other.read = false;
        other.share = false;
    }

    ~AssetRef()
    {
        if(asset != nullptr)
        {
            Asset::deref(reinterpret_cast<Asset *>(asset));
            asset = nullptr;
        }
    }

    AssetRef &operator=(AssetRef const &other)
    {
        if (this != &other)
        {
            if (asset != nullptr)
            {
                Asset::deref(reinterpret_cast<Asset *>(asset));
            }

            asset = other.asset;
            handle = other.handle;
            write = other.write;
            read = other.read;
            share = other.share;

            if (asset != nullptr)
            {
                Asset::own(reinterpret_cast<Asset *>(asset));
            }
        }

        return *this;
    }

    AssetRef &operator=(AssetRef &&other)
    {
        if (this != &other)
        {
            if (asset != nullptr)
            {
                Asset::deref(reinterpret_cast<Asset *>(asset));
            }

            asset = other.asset;
            handle = other.handle;
            write = other.write;
            read = other.read;
            share = other.share;

            other.asset = nullptr;
            other.handle = 0;
            other.write = false;
            other.read = false;
            other.share = false;
        }

        return *this;
    }

    // Returns the raw asset pointer for use in functions that need Asset*
    // Does NOT transfer or share ownership
    Asset* raw() const
    {
        return reinterpret_cast<Asset *>(asset);
    }

    // Returns just the handle - useful for lookups without ownership transfer
    uint64_t get_handle() const
    {
        return handle;
    }

    // Explicitly convert to untyped AssetRef<> with proper ownership transfer
    // This increments the ref count - the returned AssetRef owns a reference
    // Use when you need to store a typed ref in an untyped container
    AssetRef<Asset> to_untyped() const
    {
        return AssetRef<Asset>(*this);
    }


    template<typename T2>
    AssetRef<T2> casted() const
    {

        return AssetRef<T2>(*this);
    }
};
