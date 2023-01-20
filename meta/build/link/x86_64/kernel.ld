/* Tell the linker that we want an x86_64 ELF64 output file */
OUTPUT_FORMAT(elf64-x86-64)
OUTPUT_ARCH(i386:x86-64)

/* We want the symbol _start to be our entry point */
ENTRY(_start)

/* Define the program headers we want so the bootloader gives us the right */
/* MMU permissions */
/*
PHDRS
{
    text    PT_LOAD    FLAGS((1 << 0) | (1 << 2)) ; 
    rodata  PT_LOAD    FLAGS((1 << 2)) ;         
    data    PT_LOAD    FLAGS((1 << 1) | (1 << 2)) ; 
}*/

SECTIONS
{
    /* We wanna be placed in the topmost 2GiB of the address space, for optimisations */
    /* and because that is what the Limine spec mandates. */
    /* Any address in this region will do, but often 0xffffffff80000000 is chosen as */
    /* that is the beginning of the region. */
    . = 0xffffffff80000000;

    .text : {
        *(.text .text.*)
    } 

    /* Move to the next memory page for .rodata */
    . += CONSTANT(MAXPAGESIZE);

    .rodata : {
        *(.rodata .rodata.*)
    } 

    /* Move to the next memory page for .data */
    . += CONSTANT(MAXPAGESIZE);

    .data : {
        *(.data .data.*)
    } 

    .bss : {
        *(COMMON)
        *(.bss .bss.*)
        *(.note .note.*)
        *(.eh_frame .eh_frame.*)
    } 

    /* Discard .note.* and .eh_frame since they may cause issues on some hosts. */
 
}