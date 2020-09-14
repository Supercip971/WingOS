
ENTRY(main)
OUTPUT_FORMAT(elf64-x86-64)
SECTIONS
{
	. = 1M;
	__start = .;



	.text  : ALIGN(4K)
	{
		*(.text)
        
        *(.text*)
		*(.init)
		*(.fini)
		*(.ctors)
		*(.dtors)
	}

	.rodata  : ALIGN(4K)
	{
        *(.ctors)
		*(.rodata)
	}

	.data  : ALIGN(4K)
	{
		*(.data)
	}

	.bss  : ALIGN(4K)
	{
		*(COMMON)
		*(.bss)
	}

	__end = .;
}


