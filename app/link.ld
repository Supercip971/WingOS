
ENTRY(__entry_point)
OUTPUT_FORMAT(elf64-x86-64)
SECTIONS
{

	. = 4M;
	__start = .;



	.text  :
	{
		*(.text)    
    *(.text*)
		*(.init)
		*(.fini)
		*(.ctors)
		*(.dtors)
	}

	.rodata  : 
	{
    *(.ctors)
		*(.rodata)
	}

	.data  : 
	{
		*(.data)
	}

	.bss  : 
  {
		*(COMMON)
		*(.bss)
	}

	__end = .;
}


