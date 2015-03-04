ENTRY(_start)
SECTIONS
{
 . = 0x80108000;
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
