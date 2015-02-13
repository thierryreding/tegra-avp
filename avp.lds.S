ENTRY(_start)
SECTIONS
{
	. = CONFIG_TEXT_BASE;

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
