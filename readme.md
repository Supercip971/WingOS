
<div align="center">
<br>
 
![Banner](meta/doc/banner.jpg)

# Wingos

WingOS is a small hobbyist 64 bit kernel made with <3 in C++ 

[latest milestone](https://cyp.sh/blog/wingos-milestone-1/) - [blog](https://cyp.sh/blog/) - [old branch](https://github.com/Supercip971/WingOS/tree/old)
</div>

I am currently working on this rewrite. 
If you want to see the original version, you can checkout the [old branch](https://github.com/Supercip971/WingOS/tree/old). Please note that I wrote the old version when I was a beginner in C++ and OS development 3-4 years ago, and I have learned a lot since then.

If you are interested in seeing much more advanced, working, and cool projects, you should visit the [brutal](https://github.com/brutal-org/brutal) project or [skift](https://github.com/skift-org/skift).

Please be aware that this rewrite is a work in progress. And due to my current schedule and my engineering school exams, I don't have much time to work on it. I'm no longer 14 years old, and I am a lot more busy now :⁽.

## What the project is about

The project is about creating a small hobbyist kernel that is simple, easy to understand, and fun to work on.
It is not meant to be a production-ready kernel, but it doesn't mean that coding should not be taken seriously.
Everything should be done with care, and the code should aim to be fast, while being simple and easy to understand.

## Milestones

- [milestone 2 blog post](https://cyp.sh/blog/wingos-milestone-2/)
- [milestone 1 blog post](https://cyp.sh/blog/wingos-milestone-1/)

## Dependencies

- The project uses the [cutekit](https://github.com/cute-engineering/cutekit) build system.
- The project uses [limine](https://github.com/limine-bootloader/limine) as a bootloader.

## Feature list 

For now the kernel is still in development, but it has a few key features:
- is 64 bit 
- is SMP capable (multiple CPU support)
- has a basic scheduler with SMP support, priority, and CPU affinity support (and support cpu tree for NUMA systems)
- Nvme (NVMe) disk support (in userspace)
- Ext4 filesystem support (in userspace)
- VFS support (in userspace)


## Roadmap 

- [x] Boot
- [x] x86 basic support (interrupt, paging, ...) 
- [x] SMP support (apic, ...)
- [x] Scheduler (with SMP support, priority, and cpu affinity support)
- [x] Loading user space app   
- [x] User space (ring 3)
- [x] Syscalls 
- [x] Spaces and handles 
- [x] Userspace objects
    - [x] Memory management (physical)
    - [x] Memory management (virtual) 
    - [x] IPC endpoint 
- [x] Services and IPC (inter process communication) 
- [x] PCI support (scan, devices, ...) (in userspace)
- [x] Hello File!
    - [x] Nvme disk support
    - [x] service registry
    - [x] Partition support (GPT)
    - [x] Detect filesystems, and load corresponding filesystem drivers
    - [x] Filesystem support (ext4)
    - [x] VFS support
       - [x] Mount filesystems
       - [x] Open file descriptor
       - [x] Directories support/traversal
       - [x] Read files
- [x] Blocking IPC
- [x] Graphics support (VESA, framebuffer, ...)
- [ ] Input support (keyboard, mouse, ...)
- [ ] Port DOOM
- [ ] Create a shell 
- [ ] Above and beyond 

