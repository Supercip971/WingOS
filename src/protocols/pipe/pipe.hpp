#pragma once

#include "iol/wingos/asset.hpp"
#include "iol/wingos/ipc.hpp"
#include "iol/wingos/space.hpp"
#include "libcore/ds/pcring.hpp"
#include "libcore/result.hpp"
#include "math/align.hpp"
#include "wingos-headers/asset.h"
#include "wingos-headers/ipc.h"

namespace prot
{

// EMPTY DUPLEX USED FOR BUILD TIME FOR NOW, WILLE BE REWORKED LATER

template <typename T>
struct Duplex
{
    Wingos::MemoryAsset ring_phys_asset;
    Wingos::VirtualMemoryAsset ring_virt_asset;
    fc::PCRing<T> *ring = nullptr;

    static fc::Result<Duplex<T>> create(Wingos::Space creator_space, size_t ring_size = 4096)
    {
        Duplex d = {};
        d.ring_phys_asset = creator_space.allocate_physical_memory(
            math::alignUp<size_t>(ring_size, 4096));
        d.ring_virt_asset = creator_space.map_memory(d.ring_phys_asset, ASSET_MAPPING_FLAG_READ | ASSET_MAPPING_FLAG_WRITE);

        d.ring = fc::PCRing<T>::create_from_mem(d.ring_virt_asset.ptr(), ring_size);
        return d;
    }

    static fc::Result<Duplex<T>> from_mem(Wingos::Space space, Wingos::MemoryAsset const &mem_asset)
    {
        Duplex d = {};
        d.ring_phys_asset = mem_asset;
        d.ring_virt_asset = space.map_memory(d.ring_phys_asset, ASSET_MAPPING_FLAG_READ | ASSET_MAPPING_FLAG_WRITE);

        d.ring = fc::PCRing<T>::use_already_constructed_from_mem(d.ring_virt_asset.ptr());
        return d;
    }

    void send(T *ptr, size_t count)
    {
        for (size_t i = 0; i < count; i++)
        {
            ring->produce(ptr[i]);
        }
    }

    void receive(T *ptr, size_t count)
    {
        for (size_t i = 0; i < count; i++)
        {
            ptr[i] = ring->consume();
        }
    }
};

using ReceiverPipe = Duplex<uint8_t>;
using SenderPipe = Duplex<uint8_t>;

} // namespace prot
