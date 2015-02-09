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

#define TEGRA_IRAM_BASE 0x40000000

reset:
	mrs r0, cpsr
	bic r0, r0, #0x1f
	orr r0, r0, #0xd3
	msr cpsr, r0

	/* setup IRQ stack pointer */
	msr cpsr, #(0x12 | 0x80 | 0x40)
	ldr sp, =TEGRA_IRAM_BASE
	add sp, #256 << 10

	/* setup SYS stack pointer */
	msr cpsr, #(0x1f | 0x80 | 0x40)
	ldr sp, =TEGRA_IRAM_BASE
	add sp, #256 << 10
	sub sp, #4 << 10

	/* enable interrupts */
	mrs r0, cpsr
	bic r0, r0, #(0x80 | 0x40)
	msr cpsr, r0

	b start

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
	mov r13, r0 /* save r0 in r13_IRQ */
	sub r0, lr, #4 /* put return address in r0_SYS */
	mov lr, r1 /* save r1 in r14_IRQ (lr) */
	mrs r1, spsr /* put the SPSR in r1_SYS */

	msr cpsr_c, #(0x1f | 0x80) /* SYSTEM mode, IRQ disabled */
	stmfd sp!, {r0, r1} /* SPSR and PC */
	stmfd sp!, {r2-r3, r12, lr} /* AAPCS-clobbered registers */
	mov r0, sp /* make sp_SYS visible to IRQ mode */
	sub sp, sp, #8 /* make room for stacking r0 and r1 */

	msr cpsr_c, #(0x12 | 0x80) /* IRQ mode, IRQ disabled */
	stmfd r0!, {r13, r14} /* finish saving the context (r0, r1) */

	msr cpsr_c, #(0x1f | 0x80) /* SYSTEM mode, IRQ disabled */
	ldr r12, =do_irq
	mov lr, pc /* copy the return address to link register */
	bx r12 /* call the C IRQ handler (ARM/THUMB) */

	msr cpsr_c, #(0x1f | 0x80 | 0x40) /* SYSTEM mode, IRQ/FIQ disabled */
	mov r0, sp /* make sp_SYS visible to IRQ mode */
	add sp, sp, #32 /* fake unstacking 8 registers from sp_SYS */

	msr cpsr_c, #(0x12 | 0x80 | 0x40) /* IRQ mode, IRQ/FIQ disabled */
	mov sp, r0 /* copy sp_SYS to sp_IRQ */
	ldr r0, [sp, #28] /* load the saved SPSR from the stack */
	msr spsr_cxsf, r0 /* copy it into spsr_IRQ */

	ldmfd sp, {r0-r3, r12, lr}^ /* unstack all saved USER/SYSTEM registers */
	nop /* can't access barked registers immediately */
	ldr lr, [sp, #24] /* load return address from the SYS stack */
	movs pc, lr /* return restoring CPSR from SPSR */

fiq:
	bl do_fiq
