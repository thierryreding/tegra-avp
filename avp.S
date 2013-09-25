/*
 * Uses the AVP to switch on LED1 on Beaver.
 */

.globl _start
_start:
	b	reset
	b	undefined_instruction
	b	software_interrupt
	b	prefetch_abort
	b	data_abort
	b	not_used
	b	irq
	b	fiq

.balignl 16, 0xdeadbeef

reset:
	mrs r0, cpsr
	bic r0, r0, #0x1f
	orr r0, r0, #0xd3
	msr cpsr, r0

	ldr r3, TEGRA_APB_MISC_BASE
	orr r3, #0x3000
	mov r2, #0x21
	str r2, [r3, #0x12c]

	ldr r3, TEGRA_GPIO_BASE

	ldr r2, [r3, #0x20c]
	orr r2, #1
	str r2, [r3, #0x20c]

	ldr r2, [r3, #0x22c]
	orr r2, #1
	str r2, [r3, #0x22c]

	ldr r2, [r3, #0x21c]
	orr r2, #1
	str r2, [r3, #0x21c]

loop:
	
	b loop

undefined_instruction:
	b undefined_instruction

software_interrupt:
	b software_interrupt

prefetch_abort:
	b prefetch_abort

data_abort:
	b data_abort

not_used:
	b not_used

irq:
	b irq

fiq:
	b fiq

TEGRA_GPIO_BASE:
	.word 0x6000d000
TEGRA_APB_MISC_BASE:
	.word 0x70000000
