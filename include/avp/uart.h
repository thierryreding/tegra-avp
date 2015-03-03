#ifndef AVP_UART_H
#define AVP_UART_H

#include <stdbool.h>
#include <types.h>

#include <avp/clk-rst.h>

struct uart {
	unsigned long base;
	unsigned long baud;

	const struct clock_periph *clk;
	const struct reset *rst;
};

void uart_init(struct uart *uart);
bool uart_tstc(struct uart *uart);
void uart_putc(struct uart *uart, char c);
void uart_puts(struct uart *uart, const char *s);
int uart_printf(struct uart *uart, const char *format, ...) __printf(2, 3);
void uart_hexdump(struct uart *uart, void *buffer, size_t size,
		  size_t rowsize, bool ascii);
void uart_flush(struct uart *uart);

extern struct uart *debug;
extern struct uart uartd;

#endif
