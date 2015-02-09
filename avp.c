#include <avp/io.h>
#include <avp/iomap.h>
#include <avp/uart.h>

typedef unsigned char uint8_t;
typedef unsigned int size_t;

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

static void uart_puthex8(struct uart *uart, uint8_t value)
{
	uint8_t hi = (value & 0xf0) >> 4;
	uint8_t lo = (value & 0x0f) >> 0;

	if (hi >= 0xa)
		uart_putc(uart, 'a' + (hi - 0xa));
	else
		uart_putc(uart, '0' + hi);

	if (lo >= 0xa)
		uart_putc(uart, 'a' + (lo - 0xa));
	else
		uart_putc(uart, '0' + lo);
}

static uart_hexdump(struct uart *uart, void *buffer, size_t size,
		    size_t rowsize)
{
	uint8_t *ptr = buffer;
	size_t i, j;

	for (i = 0; i < size; i += rowsize) {
		for (j = 0; j < rowsize && (i + j) < size; j++) {
			uart_puthex8(uart, ptr[i + j]);
			uart_putc(uart, ' ');
		}

		uart_putc(uart, '\n');
	}

}

void start(void)
{
	unsigned long value, i, j;
	struct uart uart;

	intc_init();

	uart_init(&uart, TEGRA_UARTD_BASE, 115200);
	uart_puts(&uart, "AVP\n");

	uart_hexdump(&uart, (void *)TEGRA_IRAM_BASE, 64, 16);

	uart_printf(&uart, "AVP %s!\n", "foo");
	uart_printf(&uart, "%u\n", 42);
	uart_printf(&uart, "%u\n", 20);
	uart_printf(&uart, "%u\n",  0);
	uart_printf(&uart, "%u\n", 0xffffffff);
	uart_printf(&uart, "%u\n", -1);
	uart_printf(&uart, "%x\n", 0xaffec0fe);
	uart_printf(&uart, "%#X\n", 0xaffec0fe);

	while (1);
}
