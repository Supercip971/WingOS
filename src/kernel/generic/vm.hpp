#pragma once
#include <stdint.h>

#include "kernel/generic/asset_types.hpp"

#include "kernel/generic/asset.hpp"
#include "libcore/result.hpp"

enum VmState
{
    VMSTATE_EMBRYO = 1 << 0,
    VMSTATE_LAUNCHED = 1 << 1,
    VMSTATE_CLEAR = 1 << 2,
    VMSTATE_ACTIVE = 1 << 3,
};

/*
 * A VM structure is a physical register memory addressed by constants through set Registers.
 * Intel for example use vmwrite to write to a specific vmcs. Like register 32 may correspond to an offset of
 * 1000. Everything else is implementation specific.
 */

struct AssetVmMapping : public Asset
{
    static constexpr size_t IDENT = AssetKind::OBJECT_KIND_VM_MAPPING;

    AssetVmMapping()
        : Asset(AssetKind::OBJECT_KIND_VM_MAPPING) {};
    AssetRef<AssetMemory> memory;
    uint64_t start_address;
    uint64_t end_address;
};

struct AssetVm : public Asset
{
    static constexpr size_t IDENT = AssetKind::OBJECT_KIND_VM;

    fc::Vec<AssetVmMapping> mappings;

    AssetVm()
        : Asset(AssetKind::OBJECT_KIND_VM) {};

    size_t unique_vm_id;
    // arch specific offset

    void setRegister8(size_t byte_offset, uint64_t value);
    uint64_t getRegister8(size_t byte_offset);

    void setRegister4(size_t byte_offset, uint32_t value);
    void setRegister2(size_t byte_offset, uint16_t value);
    void setRegister1(size_t byte_offset, uint8_t value);

    fc::Result<void> launch();
    fc::Result<void> pause();
    fc::Result<void> stop();

    fc::Result<void> _internal_release();
};
