# qloader2 Bare Bones

This project will show you how to set up a simple long mode kernel using qloader2, a bootloader designed to make getting your kernel up and running as quickly as possible while supporting many useful features, such as:


* support for echfs, ext2 and fat32
* support for the linux and stivale boot protocols

This project can be build using the host compiler on most linux distros, but it's recommended you set up a [cross compiler](https://osdev.wiki/tools:compilers:gcc:x86:generic)


In order to build this you have to install [echfs](https://github.com/qword-os/echfs), and download qloader2. This can be done by adding a git submodule, if you're cloning this repository you can run `git submodule init` and `git submodule update`.

## the stivale header
This structure must be present in the `.stivalehdr` section in order for your kernel to be loaded by stivale.

```c
    struct stivale_header {
        uint64_t stack;
        // Flags
        // bit 0   0 = text mode,   1 = graphics mode
        uint16_t flags;
        uint16_t framebuffer_width;
        uint16_t framebuffer_height;
        uint16_t framebuffer_bpp;
   } __attribute__((packed));
```

 The stack member is the stack pointer that will be loaded, the flags define information on how the kernel wants to be loaded, specifically the info on whether text mode or graphics mode should be enabled and if 5 level paging should be enabled. 

## where to go from here

The first thing you should do after booting from qloader2 is loading your own [GDT](https://osdev.wiki/x86:structures:gdt)

* Load an [IDT](https://osdev.wiki/x86:structures:gdt) so that exceptions and interrupts can be handled 
* Write a physical memory allocator, a good starting point is a bitmap allocator
* Write a virtual memory manager that can map, remap and unmap pages
* Begin parsing ACPI tables, the most important one is the MADT since it contains information about the APIC
* Set up an interrupt controller such as the APIC
* Configure a timer such as the lapic timer, the PIT or the HPET
* Write a pci driver
* Add support for a storage medium, the easiest and most common ones are AHCI and NVMe
* Boot other APs in the system
* Write a scheduler

At this point you should have decided what kind of interface your os is going to provide to programs running on it, a common design that a lot of hobby operating systems use is posix, which has both pros and cons

pros:

* easier to port existing software that already runs on posix-based operating systems
* the basic parts of posix are fairly easy to implement 

cons:

* restricts you to use an already existing design
* posix is very complex and has a lot of legacy cruft that software might rely on

Other points to consider is that a lot of software tends to depend on linux-specific or glibc specific features.

Other options include adding a posix compatibility layer (with the large downside of complicating the design of your os), only implementing part of posix or just going with an entirely custom design.

In any case there are a few more features that should usually be implemented later in your os' development due to their complexity, these include support for USB(add link) and networking.
