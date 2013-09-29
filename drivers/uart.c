#include <avp/io.h>
#include <avp/types.h>
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
#define UART_LSR_THRE (1 << 5)

#define UART_MSR 0x18
#define UART_SR  0x1c

void uart_init(struct uart *uart, unsigned long base)
{
	uart->base = base;
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
