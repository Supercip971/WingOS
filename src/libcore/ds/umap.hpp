#pragma once

#include <stddef.h>

#include "libcore/str_writer.hpp"

#include "libcore/ds/vec.hpp"
#include "libcore/optional.hpp"
#include "libcore/str.hpp"
#include "libcore/type-utils.hpp"
#include "libcore/type/trait.hpp"
#include "libcore/unreachable.h"
namespace core
{

using Hash = size_t;

static inline constexpr Hash hash(size_t default_hash)
{
    return default_hash;
}

static inline constexpr Hash hash(core::Str const &str_hash)
{

    unsigned long hash = 0;

    for (size_t i = 0; i < str_hash.len(); i++)
    {
        hash = str_hash[i] + (hash << 6) + (hash << 16) - hash;
    }
    return hash;
}
static inline constexpr Hash hash(const char* str_hash)
{
    return hash(core::Str(str_hash));
}
static inline constexpr Hash hash(core::WStr const &str_hash)
{
    return hash(str_hash.view());
}
template <typename K>
concept Hashable =

    requires(K const &key) {
        {
            core::hash(key)
        } -> core::IsConvertibleTo<Hash>;
    };

template <Hashable KeyT, typename ValueT>

class UMap
{

    struct BucketEntry
    {
        KeyT key;
        ValueT value;
    };
    core::Vec<core::Vec<BucketEntry>> _buckets;

    template <IsComparable<KeyT> T>
    Hash contained_hash(T const &key) const
    {
        return hash(key) % _buckets.len();
    }

    size_t _count = 0;

    struct UMapIterator
    {
        UMap *map;
        BucketEntry *current;
        size_t _bucket_group;
        size_t _bucket_ingroup;

        UMapIterator() = default;
        UMapIterator(UMap *_map) : map(_map), current(nullptr), _bucket_group(0), _bucket_ingroup(0)
        {
            if (map->_buckets.len() == 0)
            {
                return;
            }
            while (_bucket_group < map->_buckets.len())
            {
                if (map->_buckets[_bucket_group].len() > 0)
                {
                    current = &map->_buckets[_bucket_group][_bucket_ingroup];
                    break;
                }
                _bucket_group++;
            }
        }

        static UMapIterator end()
        {
            UMapIterator it = {};
            it.current = nullptr;
            return it;
        }
        UMapIterator &operator++()
        {
            if (current == nullptr)
            {
                return *this;
            }
            _bucket_ingroup++;
            if (_bucket_ingroup >= map->_buckets[_bucket_group].len())
            {
                _bucket_group++;
                _bucket_ingroup = 0;
                while (_bucket_group < map->_buckets.len())
                {
                    if (map->_buckets[_bucket_group].len() > 0)
                    {
                        current = &map->_buckets[_bucket_group][_bucket_ingroup];
                        return *this;
                    }
                    _bucket_group++;
                }
                current = nullptr;
            }
            else
            {
                current = &map->_buckets[_bucket_group][_bucket_ingroup];
            }
            return *this;
        }

        UMapIterator &operator++(int)
        {
            if (this->current == nullptr)
            {
                return this;
            }
            auto copy = *this;
            ++(*this);
            return copy;
        }

        bool operator==(const UMapIterator &other) const
        {
            return this->current == other.current;
        }

        BucketEntry &operator*()
        {
            return *current;
        }

        BucketEntry *operator->()
        {
            return current;
        }
    };

public:
    UMap() {};
    UMap(UMap &&other) : _buckets(core::move(other._buckets)), _count(other._count) {}

    template<IsConvertibleTo<KeyT> KeyT2, IsConvertibleTo<ValueT> ValueT2>
    void insert(KeyT2 &&key, ValueT2 &&value)
    {
        if (_buckets.len() == 0)
        {
            _buckets = core::Vec<core::Vec<BucketEntry>>();
            _buckets.reserve(16);
            for (size_t i = 0; i < 16; i++)
            {
                _buckets.push(core::Vec<BucketEntry>());
            }
        }
        auto h = contained_hash(key);
#ifdef NDEBUG
        for (auto &entry : _buckets[h])
        {
            if (entry.key == key)
            {
                entry.value = value;
                return;
            }
        }
#endif
        _buckets[h].push((BucketEntry){
            (decltype(BucketEntry::key))
            core::forward<decltype(key)>(key),
(decltype(BucketEntry::value))
            core::forward<decltype(value)>(value)});
        _count++;
        maybe_rehash();
    }

    void remove(KeyT const &key)
    {
        if (_buckets.len() == 0)
        {
            return;
        }
        auto h = contained_hash(key);
        for (size_t i = 0; i < _buckets[h].len(); i++)
        {
            if (_buckets[h][i].key == key)
            {
                _buckets[h].pop(i);
                _count--;
                return;
            }
        }
    }

    template <IsComparable<KeyT> T>
    ValueT const &operator[](T const &key) const
    {
        if (_buckets.len() == 0)
        {
            unreachable$();
        }
        auto h = contained_hash(key);
        for (auto &entry : _buckets[h])
        {
            if (entry.key == key)
            {
                return entry.value;
            }
        }
        unreachable$();
    }

    template <IsComparable<KeyT> T>
    ValueT &operator[](T const &key)
    {
        if (_buckets.len() == 0)
        {
            unreachable$();
        }
        auto h = contained_hash(key);
        for (auto &entry : _buckets[h])
        {
            if (entry.key == key)
            {
                return entry.value;
            }
        }
        unreachable$();
    }

    void maybe_rehash()
    {
        if ((_count / _buckets.len()) < 4)
        {
            return;
        }

        auto old_buckets = core::move(_buckets);
        _buckets = core::Vec<core::Vec<BucketEntry>>();
        _buckets.reserve(old_buckets.len() * 2);
        for (size_t i = 0; i < old_buckets.len() * 2; i++)
        {
            _buckets.push(core::Vec<BucketEntry>());
        }
        for (auto &bucket : old_buckets)
        {

            while (bucket.len() != 0)
            {
                auto entry = bucket.pop();
                insert(core::move(entry.key), core::move(entry.value));
            }
        }
    }

    bool has(KeyT const &key) const
    {
        if (_buckets.len() == 0)
        {
            return false;
        }
        auto h = contained_hash(key);
        for (auto &entry : _buckets[h])
        {
            if (entry.key == key)
            {
                return true;
            }
        }
        return false;
    }
    core::Optional<ValueT &> find(KeyT const &key)
    {
        if (_buckets.len() == 0)
        {
            return {};
        }
        auto h = contained_hash(key);
        for (auto &entry : _buckets[h])
        {
            if (entry.key == key)
            {
                return entry.value;
            }
        }
        return {};
    }

    core::Optional<ValueT const &> find(KeyT const &key) const
    {
        if (_buckets.len() == 0)
        {
            return {};
        }
        auto h = contained_hash(key);
        for (auto &entry : _buckets[h])
        {
            if (entry.key == key)
            {
                return entry.value;
            }
        }
        return {};
    }

    UMapIterator begin()
    {
        return UMapIterator(this);
    }

    UMapIterator end()
    {
        return UMapIterator::end();
    }
};


} // namespace core
