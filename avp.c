#include <avp/io.h>
#include <avp/iomap.h>
#include <avp/uart.h>

#define TEGRA_INTC_BASE 0x60004000

void intc_init(void)
{
	unsigned long base = TEGRA_INTC_BASE;

	writel(0, base + 0x000);
	readl(base + 0x000);
}

void do_irq(void)
{
	struct uart uart;

	uart_init(&uart, TEGRA_UARTD_BASE, 115200);
	uart_puts(&uart, "IRQ\n");
}

void do_fiq(void)
{
	struct uart uart;

	uart_init(&uart, TEGRA_UARTD_BASE, 115200);
	uart_puts(&uart, "FIQ\n");
}

void start(void)
{
	unsigned long value;
	struct uart uart;

	intc_init();

	uart_init(&uart, TEGRA_UARTD_BASE, 115200);
	uart_puts(&uart, "AVP\n");

	while (1);
}
