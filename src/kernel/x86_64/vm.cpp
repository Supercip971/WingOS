
#include <stdint.h>

#include "kernel/generic/asset_types.hpp"

#include "kernel/generic/asset.hpp"
#include "kernel/generic/space.hpp"
#include "kernel/generic/vm.hpp"
#include "libcore/ds/umap.hpp"
#include "libcore/lock/lock.hpp"
#include "libcore/result.hpp"

/*
 * A VM structure is a physical register memory addressed by constants through set Registers.
 * Intel for example use vmwrite to write to a specific vmcs. Like register 32 may correspond to an offset of
 * 1000. Everything else is implementation specific.
 */

struct IntelVmxKernel
{
    AssetRef<AssetMemory> vmcs;
    AssetRef<AssetMapping> vmcs_mapped;
    size_t uid;
};

fc::Lock _VmGlobalArrayLock;
uint64_t last_uid = 16;
fc::UMap<uint64_t, IntelVmxKernel *> vms;

// arch specific offset

fc::Result<size_t> create_registered_vm(AssetRef<Space> space)
{
    (void)space;
    IntelVmxKernel *kvmx = new IntelVmxKernel();
    _VmGlobalArrayLock.lock();
    kvmx->uid = last_uid;
    last_uid++;
    vms.insert(kvmx->uid, kvmx);
    _VmGlobalArrayLock.release();

    return kvmx->uid;
}

fc::Result<AssetRef<AssetVmMapping>> Space::create_vm_mapping(AssetRef<AssetVm> vm, AssetMappingCreateParams params)
{
    (void)vm;
    auto vm_map = try$(allocate_asset<AssetVmMapping>());
    vm_map.asset->memory = params.physical_mem;

    return vm_map;
}

fc::Result<AssetRef<AssetVm>> Space::create_vm()
{
    auto vm = try$(allocate_asset<AssetVm>());
    vm.asset->unique_vm_id = try$(create_registered_vm(AssetRef<Space>(this, 0)));

    return vm;
}

void AssetVm::setRegister8(size_t byte_offset, uint64_t value)
{
    fmt::log$("VM {} set register {} = {}", this->unique_vm_id, byte_offset, value);
}

uint64_t AssetVm::getRegister8(size_t byte_offset)
{
    fmt::log$("getRegister8: byte_offset = {}, value = {}", byte_offset, 0);
    return 0;
}

void AssetVm::setRegister4(size_t byte_offset, uint32_t value)
{
    setRegister8(byte_offset, value);
}

void AssetVm::setRegister2(size_t byte_offset, uint16_t value)
{
    setRegister8(byte_offset, value);
}

void AssetVm::setRegister1(size_t byte_offset, uint8_t value)
{
    setRegister8(byte_offset, value);
}

fc::Result<void> AssetVm::launch()
{
    fmt::log$("launch");
    return {};
}

fc::Result<void> AssetVm::pause()
{
    fmt::log$("pause");
    return {};
}

fc::Result<void> AssetVm::stop()
{
    fmt::log$("stop");
    return {};
}

fc::Result<void> AssetVm::_internal_release()
{
    _VmGlobalArrayLock.lock();
    auto kvmx = vms[this->unique_vm_id];
    vms.remove(this->unique_vm_id);
    _VmGlobalArrayLock.release();

    delete kvmx;

    return {};
}
