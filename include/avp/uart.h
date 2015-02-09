#ifndef AVP_UART_H
#define AVP_UART_H

#include <types.h>

struct uart {
	unsigned long base;
};

void uart_init(struct uart *uart, unsigned long base, unsigned long baud);
void uart_putc(struct uart *uart, char c);
void uart_puts(struct uart *uart, const char *s);
int uart_printf(struct uart *uart, const char *format, ...);
void uart_hexdump(struct uart *uart, void *buffer, size_t size,
		  size_t rowsize, bool ascii);

#endif
