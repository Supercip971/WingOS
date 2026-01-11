
<div align="center">
<br>
 
![Banner](meta/doc/banner.jpg)

# Wingos

WingOS is a small hobbyist 64 bit kernel made with <3 in C++ 

[latest milestone](https://cyp.sh/blog/wingos-milestone-1/) - [blog](https://cyp.sh/blog/) - [old branch](https://github.com/Supercip971/WingOS/tree/old)
</div>

## What the project is about

The project is about creating an operating system that is easy to program for, with a simple and clean API.
- It tries to experiment with new ideas and API, ditching old and legacy concepts when possible.
- WingOS is a **microkernel** based operating system, currently targetting x86_64. It tries to be a capability based OS, with a focus on security and stability. 
- The userspace is beginning to be fleshed out, with a VFS+ext4+nvme support. 

<div align="center">
<br>

![Wingos-doom](meta/doc/wingos-doom.png)

Wingos screenshot running DOOM in userspace 

</div>


## Milestones

- [milestone 2 blog post](https://cyp.sh/blog/wingos-milestone-2/)
- [milestone 1 blog post](https://cyp.sh/blog/wingos-milestone-1/)

## Feature list 

For now the kernel is still in development, but it has a few key features:
- is 64 bit 
- is SMP capable (multiple CPU support)
- has a basic scheduler with SMP support, priority, and CPU affinity support (and support cpu tree for NUMA systems)
- Nvme (NVMe) disk support (in userspace)
- Ext4 filesystem support (in userspace)
- VFS support (in userspace)

## Dependencies

- The project uses the [cutekit](https://github.com/cute-engineering/cutekit) build system.
- The project uses [limine](https://github.com/limine-bootloader/limine) as a bootloader.


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
    - [x] Partition support (GPT)
    - [x] Filesystem support (ext4)
    - [x] VFS support
- [x] Blocking IPC
- [x] Graphics support (VESA, framebuffer, ...)
- [x] Input support (keyboard, mouse, ...)
    - [ ] Interrupt handling in userspace 
- [x] Port DOOM
- [ ] Signals 
- [ ] Create a shell 
- [ ] Above and beyond 

## Building the project

For now I don't have a complete dependency list, but you will need:
- [cutekit](https://github.com/cute-engineering/cutekit/tree/stable)
- Clang cross compiler for x86_64 
- Everything needed to build a cross compiler 

Run `bash meta/build/make_cross_compiler.sh` to build the Wingos specific version of gcc.

Now do `ck s` to start the build and run.

Do `ck p` to build ports (now no ports are buildable, do a git clone recursive to get all ports).
