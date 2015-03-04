.globl _start
_start:
	b	reset

.balignl 16, 0xdeadbeef

reset:
	ldr	x1, =0x70006000
1:
	mov	w0, #'A'
	str	w0, [x1, #0x00]

2:
	ldr	w0, [x1, #0x14]
	and	w0, w0, #0x60
	cmp	w0, #0x60
	bne	2b
	b	1b
