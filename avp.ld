ENTRY(_start)
SECTIONS
{
	. = 0x4000e000;

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
