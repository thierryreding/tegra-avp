#include <stdarg.h>

#include <avp/io.h>
#include <avp/types.h>
#include <avp/uart.h>

#define TEGRA20_UART_CLK  216000000
#define TEGRA30_UART_CLK  408000000
#define TEGRA114_UART_CLK 408000000

#define UART_THR 0x00
#define UART_RBR 0x00
#define UART_DLL 0x00

#define UART_IER 0x04
#define UART_IER_THRE (1 << 1)
#define UART_IER_RDA  (1 << 0)

#define UART_DLH 0x04

#define UART_IIR 0x08
#define UART_FCR 0x08
#define UART_LCR 0x0c
#define UART_MCR 0x10

#define UART_LSR 0x14
#define UART_LSR_THRE (1 << 5)

#define UART_MSR 0x18
#define UART_SR  0x1c

void uart_init(struct uart *uart, unsigned long base, unsigned long baud)
{
	unsigned long rate = TEGRA114_UART_CLK;
	unsigned int divisor;
	uint8_t dll, dlh;

	divisor = (rate + (baud * 8)) / (baud * 16);
	dll = (divisor >> 0) & 0xff;
	dlh = (divisor >> 8) & 0xff;

	uart->base = base;

	writel(0x00, uart->base + UART_IER);
	/* BKSE, 8 data, no parity, 1 stop bits */
	writel(0x83, uart->base + UART_LCR);
	writel(0x00, uart->base + UART_DLL);
	writel(0x00, uart->base + UART_DLH);
	/* 8 data, no parity, 1 stop bits */
	writel(0x03, uart->base + UART_LCR);
	/* RTS, DTR */
	writel(0x03, uart->base + UART_MCR);
	/* enable FIFO, clear receive and transmit FIFOs */
	writel(0x07, uart->base + UART_FCR);
	/* BKSE, 8 data, no parity, 1 stop bits */
	writel(0x83, uart->base + UART_LCR);
	writel(dll, uart->base + UART_DLL);
	writel(dlh, uart->base + UART_DLH);
	/* 8 data, no parity, 1 stop bits */
	writel(0x03, uart->base + UART_LCR);
}

void uart_putc(struct uart *uart, char c)
{
	unsigned long value;

	do {
		value = readl(uart->base + UART_LSR);
		if (value & UART_LSR_THRE)
			break;
	} while (true);

	writel(c, uart->base + UART_THR);
}

void uart_puts(struct uart *uart, const char *s)
{
	while (*s) {
		uart_putc(uart, *s);
		s++;
	}
}

int uart_printf(struct uart *uart, const char *format, ...)
{
	char buffer[80];
	va_list ap;
	int ret;

	va_start(ap, format);
	ret = vsnprintf(buffer, sizeof(buffer), format, ap);
	va_end(ap);

	uart_puts(uart, buffer);

	return ret;
}
