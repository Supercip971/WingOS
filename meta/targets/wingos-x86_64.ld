ENTRY(_start)

SECTIONS
{
    . = 4M;

    .text : ALIGN(4K)
    {
        *(.text)
        *(.text*)



		*(.dtors)
        *(.fini_array)


     }


    . = ALIGN(4K);

    .rodata : ALIGN(4K)
    {

		PROVIDE_HIDDEN (__init_array_start = .);
        KEEP (*(SORT_BY_INIT_PRIORITY(.init_array.*) ))
		KEEP(*( .init_array ));
	    PROVIDE_HIDDEN (__init_array_end = .);

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
