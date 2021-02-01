
![Banner](screen_shot/wingOS.png)

![sample](screen_shot/sample4_26_12_2020png.png)

### Building

[![Codacy Badge](https://api.codacy.com/project/badge/Grade/dbef66169c97435fb45fe7520ec891df)](https://app.codacy.com/gh/Supercip971/WingOS_x64?utm_source=github.com&utm_medium=referral&utm_content=Supercip971/WingOS_x64&utm_campaign=Badge_Grade)

for building you can take a look at the [Build guide](./Build_guide.md)

### WingOS
WingOS is a small 64 bit kernel with SMP support and a little gui system

WingOS work with the concept of 'services'... If user app want to communicate with the kernel you can do that threw kernel services instead of syscall (for exemple the file system service). User can create / replace their own services, for exemple the graphic service in app/graphic_service. 

### Implemented things :
 - com
 - gdt
 - idt
 - *pic* / ioapic
 - paging (pmm + vmm)
 - memory (thank lib alloc)
 - smp
 - multiprocessing
 - smp multiprocessing
 - ioapic timer
 - madt 
 - apic 
 - acpi
 - basic ATA driver
 - echfs support
 - program launcher (only elf64 programs for the moment)
 - process message
 - little gui system
 - ps2 mouse driver
 - ps2 keyboard driver
 - pci system
 - basic e1000 driver
 - basic rtl8139 driver
 - AHCI support
 - Sata AHCI
 - ext2fs support
 - \[insert something here]
 

### planned: 
 - terminal app
 - fat32 support
 - resizable window (gui)
 - better virtual memory management 
 - user app run in ring 3
 - better libc
 - 
