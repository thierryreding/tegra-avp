ENTRY(_start)
SECTIONS
{
	. = 0x40000000;

	.startup : {
		arch/arm/start.o(.text)
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
