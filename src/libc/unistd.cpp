
#include "stdio_fs.hpp"
// FORCED ORDER
#include <iol/iol.h>
#include <string.h>

#include "iol/wingos/space.hpp"
#include "libcore/ds/umap.hpp"
#include "libcore/shared.hpp"
#include "protocols/pipe/pipe.hpp"
#include "stdio.h"
#include "unistd.h"
#include "wingos-headers/asset.h"
#include "wingos-headers/syscalls.h"

struct PosixFile
{
    FILE *fd;

    PosixFile(FILE *_fd) : fd(_fd) {}
};

fc::UMap<int, fc::SharedPtr<PosixFile>> compat_files;

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

static void init_compat_file_if_needed()
{
    if (compat_files.count() != 0)
    {
        return;
    }

    compat_files.insert(0, fc::SharedPtr<PosixFile>::make(stdout));
    compat_files.insert(1, fc::SharedPtr<PosixFile>::make(stdin));
    compat_files.insert(2, fc::SharedPtr<PosixFile>::make(stderr));
}

int pipe(int fds[2])
{
    init_compat_file_if_needed();

    auto pipe = prot::Duplex<uint8_t>::create(Wingos::Space::self(), 8192);

    if (pipe.is_error())
    {
        return -1;
    }
    auto rpipe = new prot::Duplex<uint8_t>(pipe.unwrap());

    FILE *read_file = new FILE();
    read_file->kind = FILE_KIND_READER;
    read_file->input = rpipe;

    FILE *write_file = new FILE();
    write_file->kind = FILE_KIND_WRITER;
    write_file->output = rpipe;

    int read_fd = compat_files.count();
    int write_fd = compat_files.count() + 1;

    compat_files.insert(read_fd, fc::SharedPtr<PosixFile>::make(read_file));
    compat_files.insert(write_fd, fc::SharedPtr<PosixFile>::make(write_file));

    fds[0] = read_fd;
    fds[1] = write_fd;

    return 0;
}

ssize_t read(int fd, void *buf, size_t len)
{
    init_compat_file_if_needed();

    if (compat_files.has(fd))
    {
        auto file = compat_files[fd];
        return fread(buf, 1, len, file->fd);
    }

    return -1;
}

int close(int fd); // todo: see difference between fclose and close ?

int dup2(int oldfd, int newfd);
