ENTRY(_start)

SECTIONS
{
    . = 4M;

    .text : ALIGN(4K)
    {
        *(.text)
        *(.text*)

		*(.init)
		*(.fini)


		PROVIDE_HIDDEN (__init_array_start = .);
		KEEP (*(SORT_BY_INIT_PRIORITY(.init_array.*) SORT_BY_INIT_PRIORITY(.ctors.*)))
		KEEP (*(.init_array .ctors))
		PROVIDE_HIDDEN (__init_array_end = .);
		*(.dtors)
        *(.fini_array)
        
        
     }


    . = ALIGN(4K);

    .rodata : ALIGN(4K)
    {
       
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