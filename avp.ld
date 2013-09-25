ENTRY(_start)
SECTIONS
{
	. = 0x00000000;

	.startup : {
		avp.o(.text)
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
