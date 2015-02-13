#include <types.h>

#include <avp/intc.h>
#include <avp/io.h>
#include <avp/iomap.h>

#define FIR_CLR		0x1c

#define COP_IER		0x30
#define COP_IER_SET	0x34
#define COP_IER_CLR	0x38
#define COP_IEP_CLASS	0x3c

void intc_enable_irq(struct intc *intc, unsigned int irq)
{
	unsigned long offset = 0x100 * (irq / 32);
	unsigned int bit = irq % 32;
	uint32_t value;

	uart_printf(debug, "> %s(intc=%p, irq=%u)\n", __func__, intc, irq);

	value = readl(intc->base + offset + COP_IEP_CLASS);
	value &= ~(1 << bit);
	writel(value, intc->base + offset + COP_IEP_CLASS);

	writel(1 << bit, intc->base + offset + COP_IER_SET);

	uart_printf(debug, "< %s()\n", __func__);
}

void intc_disable_irq(struct intc *intc, unsigned int irq)
{
	unsigned long offset = 0x100 * (irq / 32);
	unsigned int bit = irq % 32;

	writel(1 << bit, intc->base + offset + COP_IER_CLR);
}

void intc_ack_irq(struct intc *intc, unsigned int irq)
{
	unsigned long offset = 0x100 * (irq / 32);
	unsigned int bit = irq % 32;

	writel(1 << bit, intc->base + offset + FIR_CLR);
}

struct intc intc = {
	.base = TEGRA_INTC_BASE,
};
