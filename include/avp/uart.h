#ifndef AVP_UART_H
#define AVP_UART_H

struct uart {
	unsigned long base;
};

void uart_init(struct uart *uart, unsigned long base);
void uart_putc(struct uart *uart, char c);
void uart_puts(struct uart *uart, const char *s);

#endif
