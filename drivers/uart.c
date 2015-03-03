#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>

#include <avp/clk-rst.h>
#include <avp/io.h>
#include <avp/iomap.h>
#include <avp/timer.h>
#include <avp/uart.h>

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
#define UART_LSR_TMTY (1 << 6)
#define UART_LSR_THRE (1 << 5)
#define UART_LSR_RDR (1 << 0)

#define UART_MSR 0x18
#define UART_SR  0x1c
#define UART_CSR 0x20
#define UART_ASR 0x3c

void uart_init(struct uart *uart)
{
	unsigned long rate = clock_get_rate(&clk_pllp);
	unsigned int divisor;
	uint8_t dll, dlh;

	clock_periph_set_source(uart->clk, 0);
	clock_periph_enable(uart->clk);
	reset_assert(uart->rst);
	reset_deassert(uart->rst);

	divisor = rate / (uart->baud * 16);
	dll = (divisor >> 0) & 0xff;
	dlh = (divisor >> 8) & 0xff;

	/* disable interrupts */
	writel(0x00, uart->base + UART_IER);

	/* reset divisor */
	writel(0x83, uart->base + UART_LCR);
	writel(0x00, uart->base + UART_DLL);
	writel(0x00, uart->base + UART_DLH);
	writel(0x03, uart->base + UART_LCR);

	/* RTS, DTR */
	writel(0x03, uart->base + UART_MCR);

	/* enable FIFO, clear receive and transmit FIFOs */
	writel(0x07, uart->base + UART_FCR);

	/* program divisor, 8 data, no parity, 1 stop bits */
	writel(0x83, uart->base + UART_LCR);
	writel(dll, uart->base + UART_DLL);
	writel(dlh, uart->base + UART_DLH);
	writel(0x03, uart->base + UART_LCR);

	/* send NUL byte */
	writel(0x00, uart->base + UART_THR);
}

bool uart_tstc(struct uart *uart)
{
	uint32_t value = readl(uart->base + UART_LSR);

	return (value & UART_LSR_RDR) != 0;
}

void uart_putc(struct uart *uart, char c)
{
	unsigned long value;

	/* some terminal programs want CRLF newlines */
	if (c == '\n')
		uart_putc(uart, '\r');

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

void uart_hexdump(struct uart *uart, void *buffer, size_t size,
		  size_t rowsize, bool ascii)
{
	uint8_t *ptr = buffer;
	size_t i, j;

	for (i = 0; i < size; i += rowsize) {
		for (j = 0; j < rowsize; j++) {
			if (j > 0)
				uart_putc(uart, ' ');

			if (i + j < size)
				uart_printf(uart, "%02x", ptr[i + j]);
			else
				uart_puts(uart, "  ");
		}

		if (ascii) {
			uart_puts(uart, " | ");

			for (j = 0; j < rowsize; j++) {
				char ch = ' ';

				if (i + j < size) {
					if (isprint(ptr[i + j]))
						ch = ptr[i + j];
					else
						ch = '.';
				}

				uart_putc(uart, ch);
			}
		}

		uart_putc(uart, '\n');
	}
}

void uart_flush(struct uart *uart)
{
	unsigned long value;

	while (true) {
		value = readl(uart->base + UART_LSR);
		if (value & UART_LSR_TMTY)
			break;
	}

	/*
	 * This seems to be necessary on Tegra114 to make sure the FIFO is
	 * completely flushed.
	 */
	udelay(1000);
}

struct uart uarta = {
	.base = TEGRA_UARTA_BASE,
	.baud = 115200,
	.clk = &clk_uarta,
	.rst = &rst_uarta,
};

struct uart uartd = {
	.base = TEGRA_UARTD_BASE,
	.baud = 115200,
	.clk = &clk_uartd,
	.rst = &rst_uartd,
};

#if defined(CONFIG_TEGRA132)
struct uart *debug = &uarta;
#else
struct uart *debug = &uartd;
#endif
