#ifndef AVP_IO_H
#define AVP_IO_H

#include <types.h>

#include <avp/uart.h>

static inline __attribute__((always_inline))
uint32_t readl(unsigned long address)
{
	return *(volatile uint32_t *)address;
}

static inline __attribute__((always_inline))
void writel(uint32_t value, unsigned long address)
{
	*(volatile uint32_t *)address = value;
}

#endif
