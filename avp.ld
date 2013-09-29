ENTRY(_start)
SECTIONS
{
	. = 0x40000000;

	.startup : {
		start.o(.text)
	}

	.text : {
		*(.text)
	}

	.data : {
		*(.data)
	}

	.bss : {
		*(.bss COMMON)
	}
}
