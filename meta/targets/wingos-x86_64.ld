ENTRY(_start)

SECTIONS
{
    . = 4M;

    .text : ALIGN(4K)
    {
        *(.text)
        *(.text*)
     }


    . = ALIGN(4K);

    .rodata : ALIGN(4K)
    {
        *(.ctors)
        *(.init_array)
        *(.fini_array)
        *(.dtors)
        *(.rodata)
    }

    . = ALIGN(4K);



    .data : ALIGN(4K)
    {
        *(.data)
    }
    . = ALIGN(4K);


    .bss : ALIGN(4K)
    {
        *(COMMON)
        *(.bss)
    }
}