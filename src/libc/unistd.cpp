#include "unistd.h"
#include <iol/iol.h>
#include <string.h>

#include "iol/wingos/space.hpp"
#include "wingos-headers/asset.h"
#include "wingos-headers/syscalls.h"

extern "C" char *getcwd(
    char *buf, size_t size)
{

    char *b = iol_get_cwd();
    if (buf == NULL)
    {
        return NULL;
    }

    size_t len = 0;
    while (b[len] != '\0' && len < size - 1)
    {
        buf[len] = b[len];
        len++;
    }

    return buf;
}

int chdir(const char *path)
{
    int res = iol_change_cwd(path);
    return res;
}

void *forked_stack;
void *temp_stack;
uintptr_t stack_ptr;

extern "C" pid_t fork_trampoline()
{
    temp_stack = (void *)((uintptr_t)malloc(16384 * 2) + 16384);
    asm volatile("mov %0, %%rsp" ::"r"(temp_stack));
    memcpy((void *)(0xcc0000000 - 16384 * 2), forked_stack, 16384 * 2);
    free(forked_stack);
    asm volatile("mov %0, %%rsp" ::"r"(stack_ptr));

    return 0;
}

extern "C" pid_t fork()
{
    asm volatile("mov %%rbp, %0" : "=r"(stack_ptr));
    // first copy stack to the new space
    forked_stack = malloc(16384 * 2);
    memcpy(forked_stack, (void *)(0xcc0000000 - 16384 * 2), 16384 * 2);

    auto subspace = Wingos::Space::self().create_space();

    auto task_asset = subspace.create_task((uintptr_t)fork_trampoline);

    // copy all memory mappings to the new space
    Wingos::Space::self().iterate_through_assets(
        [&](SyscallAssetInfo const &asset)
        {
            if (asset.returned_kind != AssetKind::OBJECT_KIND_MAPPING)
            {
                return;
            }

            auto mapping = asset.returned_info.mapping;
            auto memory = Wingos::Space::self().allocate_physical_memory(mapping.end - mapping.start);

            auto mapped_self = Wingos::Space::self().map_memory(memory, ASSET_MAPPING_FLAG_WRITE | ASSET_MAPPING_FLAG_EXECUTE);

            memcpy(mapped_self.ptr(), (void *)mapping.start, mapping.end - mapping.start);

            auto moved_memory = Wingos::Space::self().move_to(subspace, memory);
            subspace.map_memory(mapping.start, mapping.end, moved_memory, ASSET_MAPPING_FLAG_WRITE | ASSET_MAPPING_FLAG_EXECUTE);

            Wingos::Space::self().release_asset(mapped_self);
        });

    subspace.launch_task(task_asset);

    free(forked_stack);
    return 1;
}
