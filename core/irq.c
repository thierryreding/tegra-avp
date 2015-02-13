#include <errno.h>
#include <irq.h>
#include <string.h>

#include <avp/intc.h>
#include <avp/io.h>
#include <avp/iomap.h>
#include <avp/uart.h>

#define EVP_COP_IRQ_STS 0x220

struct irq_data {
	unsigned long irq;
	int (*handler)(unsigned int irq, void *data);
	void *data;
	unsigned long flags;
};

static struct irq_data irqs[16] = { { 0, }, };

static int handle_irq(unsigned int irq)
{
	int err, status = IRQ_NONE;
	unsigned int i;

	intc_disable_irq(&intc, irq);
	intc_ack_irq(&intc, irq);

	for (i = 0; i < ARRAY_SIZE(irqs); i++) {
		if (irqs[i].irq == irq) {
			err = irqs[i].handler(irqs[i].irq, irqs[i].data);
			if (err == IRQ_HANDLED) {
				status = IRQ_HANDLED;
				break;
			}
		}
	}

	intc_enable_irq(&intc, irq);

	return status;
}

void do_irq(void)
{
	unsigned int irq;
	uint32_t value;
	int err;

	value = readl(TEGRA_EVP_BASE + EVP_COP_IRQ_STS);
	irq = value & 0xff;

	uart_printf(debug, "IRQ: #%u\n", irq);

	err = handle_irq(irq);
	if (err == IRQ_NONE)
		uart_printf(debug, "WARNING: unhandled IRQ#%u\n", irq);
}

void do_fiq(void)
{
	uart_puts(debug, "FIQ\n");
}

void irq_init(void)
{
	memset(irqs, 0, sizeof(irqs));
}

int request_irq(unsigned int irq, irq_handler_t handler, void *data,
		unsigned long flags)
{
	unsigned int i;

	uart_printf(debug, "> %s(irq=%u, handler=%p, data=%p, flags=%lx)\n",
		    __func__, irq, handler, data, flags);

	for (i = 0; i < ARRAY_SIZE(irqs); i++) {
		uart_printf(debug, "  %u: %p\n", i, irqs[i].handler);

		if (irqs[i].handler == NULL) {
			uart_printf(debug, "setting up IRQ#%u at %u\n", irq, i);

			irqs[i].irq = irq;
			irqs[i].handler = handler;
			irqs[i].data = data;
			irqs[i].flags = flags;

			intc_enable_irq(&intc, irq);

			return 0;
		}
	}

	uart_printf(debug, "< %s()\n", __func__);
	return -ENOSPC;
}

void free_irq(unsigned int irq, void *data)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(irqs); i++) {
		if (irqs[i].irq == irq) {
			irqs[i].irq = 0;
			irqs[i].handler = NULL;
			irqs[i].data = NULL;
			irqs[i].flags = 0;

			intc_disable_irq(&intc, irq);
		}
	}
}
