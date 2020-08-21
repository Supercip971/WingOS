ENTRY(kernel_start)

OUTPUT_FORMAT(elf64-x86-64)
SECTIONS
{
    kernel_phys_offset = 0xffffffff80000000;
    . = kernel_phys_offset + 0x100000;

    .stivalehdr BLOCK(4K) : ALIGN(4K) 
    {
        KEEP(*(.stivalehdr))
    }

    .text BLOCK(4K) : ALIGN(4K) 
    {
        *(.text)
                *(.text*)
		*(.init)
		*(.fini)
		*(.ctors)
		*(.dtors)
    }

    .rodata BLOCK(4K) : ALIGN(4K) 
    {
       
                *(.rodata)
                *(.rodata*)
    }

    .data BLOCK(4K) : ALIGN(4K) 
    {
        *(.data)
    }

    .bss BLOCK(4K) : ALIGN(4K) 
    {
        *(.bss)
                *(COMMON)
    }


    kernel_end = .;
}
