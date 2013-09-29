#include <avp/io.h>
#include <avp/iomap.h>
#include <avp/uart.h>

void start(void)
{
	unsigned long value;
	struct uart uart;

	uart_init(&uart, TEGRA_UARTD_BASE);
	uart_puts(&uart, "AVP\n");

	while (1);
}
